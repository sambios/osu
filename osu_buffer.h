//
// Created by hsyuan on 2021-03-05.
//

#ifndef PROJECT_BUFFER_H
#define PROJECT_BUFFER_H

#include <iostream>

namespace osu {
    template <typename T, size_t fixed_size=1024/sizeof(T)+8>
    class AutoBuffer {
    public:
        typedef T value_type;

        AutoBuffer();
        explicit AutoBuffer(size_t sz);
        //! copy constructor
        AutoBuffer(const AutoBuffer<T, fixed_size> &buf);
        //! the assignment operator
        AutoBuffer<T, fixed_size>& operator = (const AutoBuffer<T, fixed_size>& buf);
        ~AutoBuffer();
        //! allocates the new buffer of size _size. if the _size is small enough, stack-allocated buffer is used
        void allocate(size_t _size);
        //! deallocates the buffer if it was dynamically allocated
        void deallocate();
        //! resizes the buffer and preserves the content
        void resize(size_t _size);
        //! returns the current buffer size
        size_t size() const;
        //! returns pointer to the real buffer, stack-allocated or heap-allocated
        inline T* data() { return ptr; }
        //! returns read-only pointer to the real buffer, stack-allocated or heap-allocated
        inline const T* data() const { return ptr; }
        //! returns pointer to the real buffer, stack-allocated or heap-allocated
        operator T* () { return ptr; }
        //! returns read-only pointer to the real buffer, stack-allocated or heap-allocated
        operator const T* () const { return ptr; }

    protected:
        //! pointer to the real buffer, can point to buf if the buffer is small enough
        T* ptr;
        //! size of the real buffer
        size_t sz;
        //! pre-allocated buffer. At least 1 element to confirm C++ standard requirements
        T buf[(fixed_size > 0) ? fixed_size : 1];
    };

    /////////////////////////////// AutoBuffer implementation ////////////////////////////////////////
    ////
    template<typename _Tp, size_t fixed_size>
    inline
    AutoBuffer<_Tp, fixed_size>::AutoBuffer() {
        ptr = buf;
        sz = fixed_size;
    }

    template<typename _Tp, size_t fixed_size>
    inline
    AutoBuffer<_Tp, fixed_size>::AutoBuffer(size_t _size) {
        ptr = buf;
        sz = fixed_size;
        allocate(_size);
    }

    template<typename _Tp, size_t fixed_size>
    inline
    AutoBuffer<_Tp, fixed_size>::AutoBuffer(const AutoBuffer<_Tp, fixed_size> &abuf) {
        ptr = buf;
        sz = fixed_size;
        allocate(abuf.size());
        for (size_t i = 0; i < sz; i++)
            ptr[i] = abuf.ptr[i];
    }

    template<typename _Tp, size_t fixed_size>
    inline AutoBuffer<_Tp, fixed_size> &
    AutoBuffer<_Tp, fixed_size>::operator=(const AutoBuffer<_Tp, fixed_size> &abuf) {
        if (this != &abuf) {
            deallocate();
            allocate(abuf.size());
            for (size_t i = 0; i < sz; i++)
                ptr[i] = abuf.ptr[i];
        }
        return *this;
    }

    template<typename _Tp, size_t fixed_size>
    inline
    AutoBuffer<_Tp, fixed_size>::~AutoBuffer() { deallocate(); }

    template<typename _Tp, size_t fixed_size>
    inline void
    AutoBuffer<_Tp, fixed_size>::allocate(size_t _size) {
        if (_size <= sz) {
            sz = _size;
            return;
        }
        deallocate();
        sz = _size;
        if (_size > fixed_size) {
            ptr = new _Tp[_size];
        }
    }

    template<typename _Tp, size_t fixed_size>
    inline void
    AutoBuffer<_Tp, fixed_size>::deallocate() {
        if (ptr != buf) {
            delete[] ptr;
            ptr = buf;
            sz = fixed_size;
        }
    }

    template<typename _Tp, size_t fixed_size>
    inline void
    AutoBuffer<_Tp, fixed_size>::resize(size_t _size) {
        if (_size <= sz) {
            sz = _size;
            return;
        }
        size_t i, prevsize = sz, minsize = std::min(prevsize, _size);
        _Tp *prevptr = ptr;

        ptr = _size > fixed_size ? new _Tp[_size] : buf;
        sz = _size;

        if (ptr != prevptr)
            for (i = 0; i < minsize; i++)
                ptr[i] = prevptr[i];
        for (i = prevsize; i < _size; i++)
            ptr[i] = _Tp();

        if (prevptr != buf)
            delete[] prevptr;
    }

    template<typename _Tp, size_t fixed_size>
    inline size_t
    AutoBuffer<_Tp, fixed_size>::size() const { return sz; }


}


#endif //PROJECT_BUFFER_H
