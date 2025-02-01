#ifndef CTZ_DAG_H_
#define CTZ_DAG_H_

#include <atomic>
#include <ciel/core/message.hpp>
#include <ciel/vector.hpp>
#include <ctz/config.hpp>
#include <ctz/waitgroup.hpp>
#include <functional>
#include <memory>

NAMESPACE_CTZ_BEGIN

// T can be reference.
template<class T>
struct DAGRunContext {
    DAGRunContext(T d)
        : data(d) {}

    // Every function parameter should be the same and stored here.
    T data;

    // Every dependent task's dependency number.
    // Their index will be stored Node::counterIndex.
    std::unique_ptr<std::atomic<size_t>[]> counters;

    template<class F>
    void invoke(F&& f) {
        f(data);
    }

}; // struct DAGRunContext

template<>
struct DAGRunContext<void> {
    std::unique_ptr<std::atomic<size_t>[]> counters;

    template<class F>
    inline void invoke(F&& f) {
        f();
    }

}; // struct DAGRunContext<void>

template<class T>
struct DAGWork {
    using type = std::function<void(T)>;

}; // struct DAGWork

template<>
struct DAGWork<void> {
    using type = std::function<void()>;

}; // struct DAGWork<void>

template<class>
class DAGBuilder;
template<class>
class DAGNodeBuilder;

template<class T>
class DAGBase {
protected:
    friend class DAGBuilder<T>;
    friend class DAGNodeBuilder<T>;

    using RunContext                            = DAGRunContext<T>;
    using Work                                  = typename DAGWork<T>::type;
    static constexpr size_t InvalidCounterIndex = -1;
    static constexpr size_t RootIndex           = 0;

    struct Node {
        Node() noexcept = default;

        Node(Work&& w) noexcept
            : work(std::move(w)) {}

        Node(const Work& w)
            : work(w) {}

        Work work;

        // If it has more than one dependency,
        // You can check upstream number of this node on RunContext::counters[counterIndex].
        size_t counterIndex = InvalidCounterIndex;

        // All downstream nodes' index.
        ciel::vector<size_t> outs;

    }; // struct Node

    // Transfer initialCounters to ctx->counters.
    void initCounters(RunContext* ctx) {
        const auto numCounters = initialCounters.size();
        ctx->counters          = std::unique_ptr<std::atomic<size_t>[]>(new std::atomic<size_t>[numCounters]);

        for (size_t i = 0; i < numCounters; ++i) {
            ctx->counters[i].store(initialCounters[i], std::memory_order_relaxed);
        }
    }

    // Every time a task is completed, notify its downstream nodes,
    // decrement their dependency number, if becoming 0 return true.
    CIEL_NODISCARD bool notify(RunContext* ctx, size_t nodeIdx) {
        Node* node = &nodes[nodeIdx];

        // Only have one dependency, steady go.
        if (node->counterIndex == InvalidCounterIndex) {
            return true;
        }

        return ctx->counters[node->counterIndex].fetch_sub(1, std::memory_order_relaxed) == 1;
    }

    void invoke(RunContext* ctx, size_t nodeIdx, WaitGroup wg) {
        Node* node = &nodes[nodeIdx];

        // Run this node's work.
        if (node->work) {
            ctx->invoke(node->work);
        }

        // Notify each one of node->outs.
        for (size_t idx : node->outs) {
            if (notify(ctx, idx)) {
                wg.add(1);

                Scheduler::schedule([this, ctx, wg, idx]() {
                    invoke(ctx, idx, wg);
                    wg.done();
                });
            }
        }
    }

    // All nodes in DAG, nodes[0] is root.
    ciel::vector<Node> nodes;
    ciel::vector<size_t> initialCounters;

}; // class DAGBase

template<class T = void>
class DAG : public DAGBase<T> {
public:
    using Builder     = DAGBuilder<T>;
    using NodeBuilder = DAGNodeBuilder<T>;

    void run(T arg) {
        typename DAGBase<T>::RunContext ctx{arg};
        this->initCounters(&ctx);
        WaitGroup wg;
        this->invoke(&ctx, this->RootIndex, wg);
        wg.wait();
    }

}; // class DAG

template<>
class DAG<void> : public DAGBase<void> {
public:
    using Builder     = DAGBuilder<void>;
    using NodeBuilder = DAGNodeBuilder<void>;

    void run() {
        typename DAGBase<void>::RunContext ctx{};
        this->initCounters(&ctx);
        WaitGroup wg;
        this->invoke(&ctx, this->RootIndex, wg);
        wg.wait();
    }

}; // class DAG<void>

template<class T>
class DAGNodeBuilder {
public:
    template<class F>
    DAGNodeBuilder then(F&& work) {
        auto node = builder->node(std::forward<F>(work));
        builder->addDependency(*this, node);
        return node;
    }

private:
    friend class DAGBuilder<T>;

    DAGNodeBuilder(DAGBuilder<T>* b, size_t i)
        : builder(b), index(i) {}

    DAGBuilder<T>* builder;
    size_t index; // Index of all nodes stored in DAGBase nodes.

}; // class DAGNodeBuilder

template<class T>
class DAGBuilder {
public:
    DAGBuilder()
        : dag(new DAG<T>) {
        dag->nodes.emplace_back();
        numIns.emplace_back(0);
    }

    DAGNodeBuilder<T> root() {
        return DAGNodeBuilder<T>{this, DAGBase<T>::RootIndex};
    }

    template<class F>
    DAGNodeBuilder<T> node(F&& work) {
        return node(std::forward<F>(work), {});
    }

    template<class F>
    DAGNodeBuilder<T> node(F&& work, std::initializer_list<DAGNodeBuilder<T>> after) {
        CIEL_ASSERT_M(numIns.size() == dag->nodes.size(), "DAGNodeBuilder vectors out of sync");

        auto index = dag->nodes.size(); // index of this new node after emplace_back, size - 1 of course.

        numIns.emplace_back(0);
        dag->nodes.emplace_back(std::forward<F>(work));

        auto node = DAGNodeBuilder<T>{this, index};

        for (auto in : after) {
            addDependency(in, node);
        }

        return node;
    }

    void addDependency(DAGNodeBuilder<T> parent, DAGNodeBuilder<T> child) {
        ++numIns[child.index];
        dag->nodes[parent.index].outs.emplace_back(child.index);
    }

    // This builder will be of invalid state after build().
    CIEL_NODISCARD std::unique_ptr<DAG<T>> build() {
        auto numNodes = dag->nodes.size();

        CIEL_ASSERT_M(numIns.size() == dag->nodes.size(), "DAGNodeBuilder vectors out of sync");

        for (size_t i = 0; i < numNodes; i++) {
            if (numIns[i] > 1) {
                auto& node = dag->nodes[i];

                node.counterIndex =
                    dag->initialCounters.size(); // index of this new node after emplace_back, size - 1 of course.
                dag->initialCounters.push_back(numIns[i]);
            }
        }

        return std::move(dag);
    }

private:
    std::unique_ptr<DAG<T>> dag;
    ciel::vector<size_t> numIns;

}; // class DAGBuilder

NAMESPACE_CTZ_END

#endif // CTZ_DAG_H_
