# CTZ

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

此项目为 C++11 的轻量级线程/纤程混合任务调度器，参考自 [Google Marl](https://github.com/google/marl)，同时做了一定简化。

我们提供了一些工具来保证任务间的协作，比如 Ticket，WaitGroup 和 Event。对于更复杂的任务还有 DAG 来专门处理。

以上工具主要解决了一个核心问题：赋予线程在当前任务阻塞时可以继续处理其它任务的能力。

## 基本用法

### Ticket

以下示例是升序打印 [1, 10000000] 内所有质数，我们以 10000 为单位切分成 1000 个任务，交给调度器进行多线程处理，然后通过 Ticket 的队列功能按顺序打印。

```cpp
// example/ticket_example.cpp

bool isPrime(int);

int main() {
    // 构造函数中可以选择要调度多少个处理器核心，allCores() 会选择所有的核心
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();   // 绑定到全局，它的意义主要在于实现 TasksInTasks，见下面 WaitGroup 示例

    // CIEL_DEFER 将语句包装进一个 lambda，并存入一个 ciel::finally 对象，
    // 该对象在离开作用域时，析构函数会执行之前存入的语句（简易的 RAII）
    CIEL_DEFER({ scheduler.unbind(); });

    ctz::TicketQueue queue;

    for (int searchBase = 1; searchBase <= 10000000; searchBase += 10000) {
        // 拿一张 Ticket，开始排队
        auto ticket = queue.take();

        ctz::schedule([=] {     // lambda 按值捕获，所有工具内部都是 std::shared_ptr，保证生命周期的正确性

            // 多个任务同时计算自己范围内的质数，存起来
            std::vector<int> primes;
            for (int i = searchBase; i < searchBase + 10000; ++i) {
                if (isPrime(i)) {
                    primes.push_back(i);
                }
            }

            // 阻塞在此，等待上一个任务完成（此线程会继续处理其它任务）
            ticket->wait();

            // 打印
            for (auto prime : primes) {
                printf("%d is prime\n", prime);
            }

            // 告诉队伍里的下一个任务可以开始了
            ticket->done();
        });
    }

    // 最后拿一张票阻塞在此，等到所有任务完成了自动退出 main 函数
    queue.take()->wait();

    return 0;
}
```

### WaitGroup

以下示例我们希望 A 任务执行到一半时，启动 B、C 任务并阻塞自己等其执行完，再继续 A。

A 先打印 1，B、C 分别打印 2、3，A 最后打印 4。结果应为 1234 或 1324.

```cpp
// example/waitgroup_example.cpp

int main() {
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();
    CIEL_DEFER({ scheduler.unbind(); });

    //   __________________________________________________________
    //  |                                                          |
    //  |               ---> [task B] ----                         |
    //  |             /                    \                       |
    //  |  [task A] -----> [task A: wait] -----> [task A: resume]  |
    //  |             \                    /                       |
    //  |               ---> [task C] ----                         |
    //  |__________________________________________________________|

    // WaitGroup 创建时指定一个数字，也就是当前任务的依赖数量
    // done() 这么多次才能放行 wait() 的任务
    ctz::WaitGroup a_wg(1);

    // 任务 A
    ctz::schedule([&str, a_wg] {
        CIEL_DEFER({ a_wg.done(); });

        puts("1");

        // 为 B C 创建一个 WaitGroup
        ctz::WaitGroup bc_wg(2);

        // 任务 B
        ctz::schedule([&str, bc_wg] {     // 在任务中调度任务，这是 Scheduler::bind 到全局的意义
            CIEL_DEFER({ bc_wg.done(); });
            puts("2");
        });

        // 任务 C
        ctz::schedule([&str, bc_wg] {
            CIEL_DEFER({ bc_wg.done(); });
            puts("3");
        });

        // 等待 B C 完成
        bc_wg.wait();

        puts("4");
    });

    // 等待任务 A 完成
    a_wg.wait();
}
```

### Event

以下示例我们希望任务 A 在任务 B、C 任意一个完成时就可启动。

A、B、C 分别打印自己，结果应为 BAC BCA CAB CBA 中任意一种。

```cpp
// example/event_example.cpp

int main() {
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();
    CIEL_DEFER({ scheduler.unbind(); });

    ctz::Event B;
    ctz::Event C;
    
    auto il = {B, C};

    ctz::Event A = ctz::Event::any(il.begin(), il.end());

    ctz::schedule([=] {
        A.wait();   // 等待 B、C 任意一个完成后 signal()
        puts("A");
    });

    ctz::schedule([=] {
        CIEL_DEFER({ B.signal(); });
        puts("B");
    });

    ctz::schedule([=] {
        CIEL_DEFER({ C.signal(); });
        puts("C");
    });
}
```

### DAG

```cpp
// test/dag_test.cpp

/*
          /--> [A0] --\        /--> [C0] --\        /--> [E0] --\
 [root] --|--> [A1] --|-->[B]--|--> [C1] --|-->[D]--|--> [E1] --|-->[F]
                               \--> [C2] --/        |--> [E2] --|
                                                    \--> [E3] --/
*/
TEST(DAGTest, DAGFanOutFanIn) {
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();
    CIEL_DEFER({ scheduler.unbind(); });

    ctz::DAG<Data&>::Builder builder;

    auto root = builder.root();
    auto a0 = root.then([](Data& data) { data.push("A0"); });
    auto a1 = root.then([](Data& data) { data.push("A1"); });

    auto b = builder.node([](Data& data) { data.push("B"); }, {a0, a1});

    auto c0 = b.then([](Data& data) { data.push("C0"); });
    auto c1 = b.then([](Data& data) { data.push("C1"); });
    auto c2 = b.then([](Data& data) { data.push("C2"); });

    auto d = builder.node([](Data& data) { data.push("D"); }, {c0, c1, c2});

    auto e0 = d.then([](Data& data) { data.push("E0"); });
    auto e1 = d.then([](Data& data) { data.push("E1"); });
    auto e2 = d.then([](Data& data) { data.push("E2"); });
    auto e3 = d.then([](Data& data) { data.push("E3"); });

    builder.node([](Data& data) { data.push("F"); }, {e0, e1, e2, e3});

    auto dag = builder.build();

    Data data;
    dag->run(data);
}
```

## 内部实现细节

### Fiber

协程的一些基础知识见：[CoroutineExplanation_CN.md](./CoroutineExplanation_CN.md)

使用纤程作为有栈对称协程实现，它可以在执行过程中保存目前状态并切换至其它任意一个纤程，这是整个项目的基石。

纤程与操作系统课上学到的用户级线程类似，切换时无需像线程一样切换至内核态，开销更小。x86 线程之间的上下文切换通常需要数千个 CPU 周期，而纤程切换则只需要不到 100 个。

以下示例说明了从主纤程到 C、B、A、C、B、A 再回到主纤程的切换逻辑。

```cpp
// test/osfiber_test.cpp

TEST(OSFiberTest, OSFiber2) {
    std::string str;
    auto main = ctz::OSFiber::createFiberFromCurrentThread();

    std::unique_ptr<ctz::OSFiber> fiberA, fiberB, fiberC;

    fiberC = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "C";
        fiberC->switchTo(fiberB.get());
        
        str += "C";
        fiberC->switchTo(fiberB.get());
    });

    fiberB = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "B";
        fiberB->switchTo(fiberA.get());
        
        str += "B";
        fiberB->switchTo(fiberA.get());
    });

    fiberA = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "A";
        fiberA->switchTo(fiberC.get());
        
        str += "A";
        fiberA->switchTo(main.get());
    });

    main->switchTo(fiberC.get());

    ASSERT_EQ(str, "CBACBA");
}
```

纤程对象内存储着三个变量：

```cpp
ctz_fiber_context context{};
std::function<void()> target;
void* stack{nullptr};
```

context 存储着寄存器上下文，target 是需要执行的任务，stack 是一块在构造函数中 malloc 堆分配出来的模拟栈。这样纤程在切换时就可将目前状态全部保存下来。

在构造函数中会调用以下函数，设置相关的寄存器和栈顶指针：

```cpp
void ctz_fiber_set_target(struct ctz_fiber_context* ctx,
                         void* stack,
                         uint32_t stack_size,
                         void (*target)(void*),
                         void* arg) {

#if defined(linux) || defined(__linux) || defined(__linux__)
    if (__hwasan_tag_memory && __hwasan_tag_pointer) {
        stack = __hwasan_tag_pointer(stack, 0);
        __hwasan_tag_memory(stack, 0, stack_size);
    }
#endif
    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
    ctx->LR = (uintptr_t)&ctz_fiber_trampoline;
    ctx->r0 = (uintptr_t)target;
    ctx->r1 = (uintptr_t)arg;
    ctx->SP = ((uintptr_t)stack_top) & ~(uintptr_t)15;
}
```

在切换纤程时的 switchTo(OSFiber*) 中会调用 ctz_fiber_swap(ctz_fiber_context* from, const ctz_fiber_context* to)：

```asm
// void ctz_fiber_swap(ctz_fiber_context* from, const ctz_fiber_context* to)
// x0: from
// x1: to
.text
.global CTZ_ASM_SYMBOL(ctz_fiber_swap)
.align 4
CTZ_ASM_SYMBOL(ctz_fiber_swap):

    // Save context 'from'

    PAUTH_SIGN_SP

    // Store special purpose registers
    str x16, [x0, #CTZ_REG_r16]
    str x17, [x0, #CTZ_REG_r17]
    str x18, [x0, #CTZ_REG_r18]

    // Store callee-preserved registers
    str x19, [x0, #CTZ_REG_r19]
    str x20, [x0, #CTZ_REG_r20]
    str x21, [x0, #CTZ_REG_r21]
    str x22, [x0, #CTZ_REG_r22]
    str x23, [x0, #CTZ_REG_r23]
    str x24, [x0, #CTZ_REG_r24]
    str x25, [x0, #CTZ_REG_r25]
    str x26, [x0, #CTZ_REG_r26]
    str x27, [x0, #CTZ_REG_r27]
    str x28, [x0, #CTZ_REG_r28]
    str x29, [x0, #CTZ_REG_r29]

    str d8,  [x0, #CTZ_REG_v8]
    str d9,  [x0, #CTZ_REG_v9]
    str d10, [x0, #CTZ_REG_v10]
    str d11, [x0, #CTZ_REG_v11]
    str d12, [x0, #CTZ_REG_v12]
    str d13, [x0, #CTZ_REG_v13]
    str d14, [x0, #CTZ_REG_v14]
    str d15, [x0, #CTZ_REG_v15]

    // Store sp and lr
    mov x2, sp
    str x2,  [x0, #CTZ_REG_SP]
    str x30, [x0, #CTZ_REG_LR]

    // Load context 'to'
    mov x7, x1

    // Load special purpose registers
    ldr x16, [x7, #CTZ_REG_r16]
    ldr x17, [x7, #CTZ_REG_r17]
    ldr x18, [x7, #CTZ_REG_r18]

    // Load callee-preserved registers
    ldr x19, [x7, #CTZ_REG_r19]
    ldr x20, [x7, #CTZ_REG_r20]
    ldr x21, [x7, #CTZ_REG_r21]
    ldr x22, [x7, #CTZ_REG_r22]
    ldr x23, [x7, #CTZ_REG_r23]
    ldr x24, [x7, #CTZ_REG_r24]
    ldr x25, [x7, #CTZ_REG_r25]
    ldr x26, [x7, #CTZ_REG_r26]
    ldr x27, [x7, #CTZ_REG_r27]
    ldr x28, [x7, #CTZ_REG_r28]
    ldr x29, [x7, #CTZ_REG_r29]

    ldr d8,  [x7, #CTZ_REG_v8]
    ldr d9,  [x7, #CTZ_REG_v9]
    ldr d10, [x7, #CTZ_REG_v10]
    ldr d11, [x7, #CTZ_REG_v11]
    ldr d12, [x7, #CTZ_REG_v12]
    ldr d13, [x7, #CTZ_REG_v13]
    ldr d14, [x7, #CTZ_REG_v14]
    ldr d15, [x7, #CTZ_REG_v15]

    // Load parameter registers
    ldr x0, [x7, #CTZ_REG_r0]
    ldr x1, [x7, #CTZ_REG_r1]

    // Load sp and lr
    ldr x30, [x7, #CTZ_REG_LR]
    ldr x2,  [x7, #CTZ_REG_SP]
    mov sp, x2

    PAUTH_AUTH_SP

    ret
```

### Scheduler

一个天真的线程池实现，可能是一个全局的线程池和任务队列，也只有对应的一把锁。所有线程会争抢这把锁来取得自己的任务，这样锁竞争会非常激烈。

为了减少锁竞争的性能损失，我们将任务队列分片绑定到每个工人线程上，再用工作窃取算法（也就是某线程自己的任务队列空了以后会去其它线程的任务队列尝试拿）来平衡每个线程的工作量。这样每个任务队列的锁竞争就被缩小到：Scheduler 派发任务，工人线程拿任务，被别的工人线程偷任务。

一个 Scheduler 在构造的时候需要传入一直 SchedulerConfig，里面包含了要开的线程数量和 Fiber 模拟栈大小（见下节）。它的内部成员为：

```cpp
std::vector<std::unique_ptr<Worker>> workers;
std::atomic<size_t> workNum{0};
std::atomic<size_t> index{0};
```

workers 根据线程数量动态 reserve 对应容量然后构造一批线程（每个工人持有者一个线程）；

workNum 是入队并还没被做完的任务量，在 Scheduler 析构时会有一个自旋锁来忙等待 workNum 归零；

index 是新任务入队时选择的 workers 下标，每次原子地自增，结果对线程量（workers.size()）取余，便能得到需要入队的那个工人线程。并且无符号数自增到最大值后会回到 0，所以不用担心溢出问题。之后取得工人内部的锁并且将任务加入工人内部的

```cpp
std::queue<std::function<void()>> queuedTasks
```

注：使用 std::unique_ptr&lt;Worker> 而不是 Worker 纯粹是因为语义问题：Worker 不允许复制和移动构造，而这使其不符合容器的元素条件。其它地方也是如此。

### Worker

每个工人持有着一个线程，当它开始工作时，会切换到一个工人协程，并启动

```cpp
[[noreturn]] void run() noexcept;
```

在这个函数中它会一直处理队列中的任务，如果自己的队列空了，会尝试去偷其它工人的队列，如果再没有，则阻塞自己等待新任务入队时被唤醒。

当 Scheduler 要关闭并且结束工人线程时，会向任务队列派发最后一个任务：切回到主纤程。为了这个特殊任务不被偷，偷任务的判断条件是队列中任务不止一个。

### ConditionVariable

ConditionVariable 是线程在当前任务阻塞时可继续处理其它任务的原因。

我们重写了 std::condition_variable 的逻辑，使得 wait() 和 notify() 可以将当前阻塞任务保存起来并切换，等待条件满足时将其重新入队。它被用于上述一系列工具中。

例：

当在任务中调用 WaitGroup::wait()，它会拿到 thread_local 的当前线程工人指针，将目前这个任务存在 ConditionVariable::waitingTasks 中，切换至另一个待处理任务（纤程）。

WaitGroup::notify() 后会将 ConditionVariable::waitingTasks 中的任务重新入队（每个 Fiber 内部存储着自己所属的工人指针，所以可以找到原属的位置）。

而在 Scheduler 外的 wait 调用会委托到 std::condition_variable 上。
