#pragma once

// Don't use __unroll_for directly, this is a helper class for function unroll_for
template<size_t BEGIN, size_t END, size_t STEP, typename TF>
class __unroll_for
{
public:
    __forceinline static void run(TF& func)
    {
        func(BEGIN);
        unroll_for<BEGIN+STEP, END, STEP>(func);
    }
};

template<size_t END, size_t STEP, typename TF>
class __unroll_for<END, END, STEP, TF>
{
public:
    __forceinline static void run(TF&) { }
};

// unroll_for function:
// Used to unroll for loop at compile-time. Caller should supply BEGIN, END(exclusive), STEP similiar to
// linear for-statement, also supply an array (in TA type) and a functor operator on the array.
// The functor has interface like:
//   void operator()(TA& block, size_t j)
// where j is the iterator index into the the array
// Note: if the functor need context access, please use functor constructor and private data member to
// pass and hold it.
template<size_t BEGIN, size_t END, size_t STEP, typename TF>
__forceinline void unroll_for(TF& func)
{
    __unroll_for<BEGIN, END, STEP, TF>::run(func);
}
