namespace ModernCPP {
	template < typename T >
	class atomic {
	private:
		T obj;

	public:
		atomic( const T& val ) {
			__sync_synchronize();   // (I)
		    obj = val;
			__sync_synchronize();   // (II)
		}

		operator T() {
			__sync_synchronize();   // (I)
			T tmp=obj;
			__sync_synchronize();   // (II)
			return tmp;
		}

		atomic<T>& operator=( const T& val ) {
			__sync_synchronize();   // (I)
		    obj = val;
			__sync_synchronize();   // (II)
		    return *this;
		}

		T Load(){
			__sync_synchronize();   // (I)
			T val = obj;
			__sync_synchronize();   // (II)
			return val;
		}
		void Store(const T& val){
			__sync_synchronize();   // (I)
		    obj = val;
			__sync_synchronize();   // (II)
		}

		T operator++() {
		    return __sync_add_and_fetch( &obj, (T)1 );
		}

		T operator++( int ) {
		    return __sync_fetch_and_add( &obj, (T)1 );
		}

		T operator+=(const T& val ) {
		    return __sync_add_and_fetch( &obj, val );
		}

		T operator--() {
		    return __sync_sub_and_fetch( &obj, (T)1 );
		}

		T operator--( int ) {
		    return __sync_fetch_and_sub( &obj, (T)1 );
		}

		T operator-=(const T& val ) {
		    return __sync_sub_and_fetch( &obj, val );
		}

		// Perform an atomic CAS operation
		// returning the value before the operation
		T exchange( const T& oldVal, const T& newVal ) {
		    return __sync_val_compare_and_swap( &obj, oldVal, newVal );
		}

	};
};

