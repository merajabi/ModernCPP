#ifndef _ModernCPP_atomic_H_
#define _ModernCPP_atomic_H_

#include "typetraits.h"
#include "mutex.h"

// ***************** Discussion *******************
// atomic in C+11 are implemented in Sequentially-consistent ordering by default.
// If we implement the operation just by using __sync builtin in C, we have implemented Relaxed ordering,
// so inorder to be consistent with C++11 we should either use full fence or acquire&release barrier.
// but acquire&release may not be available on all architecture, 
// after all, if you are coding on x86/x64 or you have modern compiler with c++11 support you do not need this library at all!
// so I will go with implementing full fence for basic types to be safe
// for general object we should acquire lock



// ***************** List of Atomic-Builtins in gcc along their X86-64 ASM codes *******************
// This builtin issues a full memory barrier.
// __sync_synchronize()
// {
//	mfence
// }

// This builtin is atomic exchange operation. It writes value into *ptr, and returns the previous contents of *ptr. This builtin is "acquire barrier"
// __sync_lock_test_and_set(type *ptr, type value);
// {
//	mov     eax, 10
//	xchg    eax, DWORD PTR obj[rip]; // note lock is default in xchg with memory
//	nop;
// }

// This builtin releases the lock acquired by __sync_lock_test_and_set. This builtin is release barrier
// __sync_lock_release(&obj); 
// {
//	mov     eax, 0
//	mov     DWORD PTR obj[rip], eax
//	nop
// }

// __sync_bool_compare_and_swap
// __sync_val_compare_and_swap
// {
//	lock cmpxchg    BYTE PTR bo[rip], dl
//	lock cmpxchg    DWORD PTR obj[rip], edx
// }

// __sync_fetch_and_add
// __sync_add_and_fetch
// __sync_fetch_and_sub
// __sync_sub_and_fetch
// {
//	lock add        DWORD PTR obj[rip], eax
//	nop
// }

// __sync_fetch_and_or
// __sync_or_and_fetch
// __sync_fetch_and_and
// __sync_and_and_fetch
// __sync_fetch_and_xor
// __sync_xor_and_fetch
// {
//	lock or        DWORD PTR obj[rip], eax
//	lock and        DWORD PTR obj[rip], eax
//	lock xor        DWORD PTR obj[rip], eax
//	nop
// }

// __sync_fetch_and_nand
// __sync_nand_and_fetch

namespace ModernCPP {
	// it's general class, but it's not complete yet, 
	// we need lock for general object
	// and we should implement specilization for basic types
	template < typename T >
	class atomic {
	private:
		mutex m;
		T obj;

		atomic( const atomic& val );
		atomic& operator = ( const atomic& val );
		bool compare_exchange_weak( const T& oldVal, const T& newVal ); // it does not mean for this general case
	public:
		atomic():obj(){};
		atomic( const T& val ) {
			lock_guard<mutex> lk(m);
			obj = val;
		}

		T operator=( const T& val ) {
			lock_guard<mutex> lk(m);
			obj = val;
		    return val;
		}

		operator T() {
			lock_guard<mutex> lk(m);
			return obj;
		}

		T load(){
			lock_guard<mutex> lk(m);
			return obj;
		}
		void store(const T& val){
			lock_guard<mutex> lk(m);
			obj = val;
		}

		T operator++() {
			lock_guard<mutex> lk(m);
			return ++obj;
		}

		T operator++( int ) {
			lock_guard<mutex> lk(m);
			return obj++;
		}

		T operator+=(const T& val ) {
			lock_guard<mutex> lk(m);
			obj += val;
			return obj;
		}

		T operator--() {
			lock_guard<mutex> lk(m);
			return --obj;
		}

		T operator--( int ) {
			lock_guard<mutex> lk(m);
			return obj--;
		}

		T operator-=(const T& val ) {
			lock_guard<mutex> lk(m);
			obj -= val;
			return obj;
		}

		bool operator==(const T& val ) {
			lock_guard<mutex> lk(m);
			return (obj == val);
		}
		bool operator!=(const T& val ) {
			lock_guard<mutex> lk(m);
			return (obj != val);
		}
		
		bool compare_exchange_strong( const T& oldVal, const T& newVal ) {
			lock_guard<mutex> lk(m);
			bool result = false;
			if(obj == oldVal){
				obj = newVal;
				result = true;
			}
			return result;
		}
	};
/*
	template <>
	class atomic<int,long,void* ... > {
	private:
		mutex m;
		T obj;
		atomic( const atomic& val );
		atomic& operator= ( const atomic& val );
	public:
		atomic():obj{};
		atomic( const T& val ) {
			lock_guard<mutex> lk(m);
			obj = val;
		}

		T operator=( const T& val ) {
			lock_guard<mutex> lk(m);
			obj = val;
		    return val;
		}

		operator T() {
			lock_guard<mutex> lk(m);
			T val=obj;
			return val;
		}

		T load(){
			lock_guard<mutex> lk(m);
			T val = obj;
			return val;
		}
		void store(const T& val){
			lock_guard<mutex> lk(m);
			obj = val;
		}

		T operator++() {
			__sync_synchronize();   // (I) mfence
		    T val = __sync_add_and_fetch( &obj, (T)1 ); // lock xadd
			__sync_synchronize();   // (II) mfence
			return val;
		}

		T operator++( int ) {
			__sync_synchronize();   // (I) mfence
		    T val = __sync_fetch_and_add( &obj, (T)1 ); // lock xadd
			__sync_synchronize();   // (II) mfence
			return val;
		}

		T operator+=(const T& val ) {
			__sync_synchronize();   // (I) mfence
		    return __sync_add_and_fetch( &obj, val ); // lock xadd
			__sync_synchronize();   // (II) mfence
		}

		T operator--() {
			__sync_synchronize();   // (I) mfence
		    return __sync_sub_and_fetch( &obj, (T)1 ); // lock xadd
			__sync_synchronize();   // (II) mfence
		}

		T operator--( int ) {
			__sync_synchronize();   // (I) mfence
		    return __sync_fetch_and_sub( &obj, (T)1 ); // lock xadd
		}

		T operator-=(const T& val ) {
			__sync_synchronize();   // (I) mfence
		    return __sync_sub_and_fetch( &obj, val ); // lock xadd
			__sync_synchronize();   // (II) mfence
		}

		bool operator==(const T& val ) {
		  //  return __sync_bool_compare_and_swap( &obj, val, val); // lock cmpxchg    BYTE PTR bo[rip], dl
			return compare_exchange_weak(val,val);
		}
		bool operator!=(const T& val ) {
		    return !__sync_bool_compare_and_swap( &obj, val, val);// lock cmpxchg    BYTE PTR bo[rip], dl
			return compare_exchange_weak(val,val);
		}

		// Perform an atomic CAS operation
		// returning the value before the operation
		bool compare_exchange_weak( const T& oldVal, const T& newVal ) {
			__sync_synchronize();   // (I) mfence
		    bool result = ( __sync_val_compare_and_swap( &obj, oldVal, newVal ) == oldVal );// lock cmpxchg    DWORD PTR obj[rip], edx
			__sync_synchronize();   // (II) mfence
			return result;
		}
	};
*/
}

#endif //_ModernCPP_atomic_H_

