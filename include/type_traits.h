#ifndef _ModernCPP_meta_H_
#define _ModernCPP_meta_H_

namespace ModernCPP {

	template<bool B, class T = void>
	struct enable_if {};

	template<class T>
	struct enable_if<true, T> { typedef T type; };

	template < typename A, typename B >
	struct is_same { static const bool value = false; };

	template < typename A >
	struct is_same<A, A> { static const bool value = true; };

	template< class T > 
	struct remove_const          { typedef T type; };

	template< class T > 
	struct remove_const<const T> { typedef T type; };
	 
	template< class T > 
	struct remove_volatile             { typedef T type; };

	template< class T > 
	struct remove_volatile<volatile T> { typedef T type; };

	template< class T >
	struct remove_cv { 
		typedef typename ModernCPP::remove_volatile<typename ModernCPP::remove_const<T>::type>::type type;
	};

	template< class T >	struct add_cv { typedef const volatile T type; };
	template< class T > struct add_const { typedef const T type; };
	template< class T > struct add_volatile { typedef volatile T type; };

	template< class T > struct remove_reference      {typedef T type;};
	template< class T > struct remove_reference<T&>  {typedef T type;};
	//template< class T > struct remove_reference<T&&> {typedef T type;};

	template< class T >
	struct remove_cvref {
		typedef typename ModernCPP::remove_cv<typename ModernCPP::remove_reference<T>::type>::type type;
	};

	template< class T >
	T& ref(T& x){
		return x;
	};

};
#endif //_ModernCPP_meta_H_

