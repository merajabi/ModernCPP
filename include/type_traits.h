#ifndef _ModernCPP_meta_H_
#define _ModernCPP_meta_H_
namespace ModernCPP {

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

template < typename A, typename B >
struct is_same {
    static const bool value = false;
};

template < typename A >
struct is_same<A, A> {
    static const bool value = true;
};

template< class T >
T& ref(T& x){
	return x;
};

};
#endif //_ModernCPP_meta_H_

