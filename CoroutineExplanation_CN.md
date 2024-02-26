协程就是一个能在中途保存状态 return，并且下次能在之前保存的状态处恢复的函数。

为了实现这种功能，我们自然是需要一些东西来存储函数中的变量，这样才能恢复。

考虑一个最简单的想法：

```cpp
// example/stackless_coroutine_example.cpp

class func {
private:
    const std::string info = "Coroutine at level ";
    size_t flag = 0;

public:
    std::string operator()() noexcept {
        switch (flag) {
            case 0 :
                return info + static_cast<char>('0' + flag++);

            case 1 :
                return info + static_cast<char>('0' + flag++);

            case 2 :
                return info + static_cast<char>('0' + flag++);

            case 3 :
                return info + static_cast<char>('0' + flag++);

            case 4 :
                return info + static_cast<char>('0' + flag++);

            case 5 :
                return info + static_cast<char>('0' + flag++);

            default :
                return "No more levels";
        }
    }

};  // struct func

int main() {
    func f1;
    for (size_t i = 0; i < 3; ++i) {
        std::cout << f1() << '\n';
    }

    std::cout << '\n';

    func f2;
    for (size_t i = 0; i < 7; ++i) {
        std::cout << f2() << '\n';
    }
}
```

输出为：

```
Coroutine at level 0
Coroutine at level 1
Coroutine at level 2

Coroutine at level 0
Coroutine at level 1
Coroutine at level 2
Coroutine at level 3
Coroutine at level 4
Coroutine at level 5
No more levels
```

可以粗浅地理解为，当我们在函数中使用了协程关键词（如 co_yield），编译器会自动将其转为一个类似上面的结构体，并且将状态保存起来，这便是无栈协程。

可以注意到这种方式对编译器的依赖会比较高。而且一般来说这种协程的切换只能回到调用方，而不能随便切换到任意一个地方，这便是非对称协程。

而本项目中的协程便是相反的有栈对称协程。对称意味着它可以跳到任意一个协程，功能更加强大。有栈是因为它需要提前分配一块非常大的堆内存来模拟函数调用栈，然后还需要一个寄存器上下文结构体来存储对应的寄存器。显然有栈协程不可避免的造成了空间浪费。