#include "operators/softmax_loss.h"

namespace mobula {

template <typename T>
MOBULA_KERNEL SoftmaxLossForward(
    const int nthreads,
    const T *data,
    const int num_classes,
    const int outer_size,
    const int inner_size,
    T *probs) {
    KERNEL_LOOP(index, nthreads) {
        int j = get_middle_loop_offset(index, num_classes, inner_size);
        const T *data_i = data + j;
        T *probs_i = probs + j;
        // get maximum
        T max_val;
        mobula_reduce(std::max<T>, data_i, num_classes, inner_size, &max_val);
        // exp(x - max(x))
        mobula_map([&max_val](const T &a){return std::exp(a - max_val);}, data_i, num_classes, inner_size, probs_i);
        // sum
        T sum_val;
        mobula_reduce([](const T &a, const T &b){return a + b;}, probs_i, num_classes, inner_size, &sum_val);
        // result
        mobula_map([&sum_val](const T &a){return a / sum_val;}, probs_i, num_classes, inner_size);
    }
}

} // namespace mobula

void softmax_loss_forward(
    const DType *data,
    const int num_classes,
    const int outer_size,
    const int inner_size,
    DType *probs) {
    const int nthreads = outer_size * inner_size;
    KERNEL_RUN(SoftmaxLossForward<DType>, nthreads)(nthreads, data, num_classes, outer_size, inner_size, probs);
}
