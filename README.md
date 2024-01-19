# CTZ

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This project is a hybrid thread / fiber task scheduler written in C++11 only for educational purposes, highly inspired by [Google Marl](https://github.com/google/marl), and some simplifications were made as well.

We provide a set of tools to guarantee the works' order users expected, like Ticket, WaitGroup and Event. And we have DAG for even more complex works. You can check the tests to see how each one of them works.

## Fiber

We use Fiber as the implementation of stackful symmetric coroutines.

The Fiber class have three objects inside,

```cpp
ctz_fiber_context context{};
std::function<void()> target;
void* stack{nullptr};
```

context stores the registers, target is the task needed to run, and stack will malloc a block of memory to simulate as fiber's stack.

We will set them in constructors.

```cpp
void ctz_fiber_set_target(struct ctz_fiber_context* ctx, void* stack, uint32_t stack_size, void (*target)(void*),
                          void* arg) {
    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
    ctx->LR              = (uintptr_t)&ctz_fiber_trampoline;
    ctx->r0              = (uintptr_t)target;
    ctx->r1              = (uintptr_t)arg;
    ctx->SP              = ((uintptr_t)stack_top) & ~(uintptr_t)15;
}
```

We can use

```cpp
void switchTo(OSFiber*);
```

to jump to whatever other fibers created by this thread as you like. And it is the strong foundation of this project.

## ConditionVariable

How can a thread continue processing other tasks when the current task block itself?

We have our own ConditionVariable, which changes the internal mechanism of wait() and notify(). And it's held inside by our tools like Ticket, WaitGroup and Event.

E.g. When you call WaitGroup::wait() in task, it will call ConditionVariable::wait(), get the thread_local current worker pointer, store the current Task in ConditionVariable::waitingTasks, switchTo another fiber.

And for those calls outside of scheduler, ConditionVariable will delegate them to std::condition_variable.
