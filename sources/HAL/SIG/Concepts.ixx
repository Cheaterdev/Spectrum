export module HAL:Concepts;
import Core;

export
{

	template<typename T> concept HasData =
		requires (T t) {
		t.data;
	};


	template<typename T> concept HasBindless =
		requires (T t) {
		t.bindless;
	};


	template<typename T> concept HasSRV =
		requires (T t) {
		t.srv;
	};

	template<typename T> concept HasUAV =
		requires (T t) {
		t.uav;
	};

	template<typename T> concept HasSMP =
		requires (T t) {
		t.smp;
	};

	template<typename T> concept HasCB =
		requires (T t) {
		t;
	};

	template<typename T> concept HasRTV =
		requires (T t) {
		t.rtv;
	};

	template<typename T> concept HasDSV =
		requires (T t) {
		t.dsv;
	};

	template<typename T> concept HasSlot =
		requires () {
		T::Slot;
	};

	
	template<typename T> concept HasID =
		requires () {
		T::ID;
	};
	template<typename T> concept TableHasSRV =
		requires (T t) {
		t.SRV;
	};

	template<typename T> concept TableHasUAV =
		requires (T t) {
		t.UAV;
	};

	template<typename T> concept TableHasSMP =
		requires (T t) {
		t.SMP;
	};

	template<typename T> concept TableHasCB =
		requires (T t) {
		t.CB;
	};



	template<typename T> concept HasCBType =
		requires () {
		T;
	};

	template<class T>
	concept IsEnumClass = requires () { typename T::EnumClass; };

		template<class T>
	concept CompilableClass = requires () { typename T::Compiled; };

	template<class T>
	struct _Underlying
	{
		using TYPE = T;
	};

	template<IsEnumClass T>
	struct _Underlying<T>
	{
		using TYPE = typename T::EnumClass;
	};
template<CompilableClass T>
	struct _Underlying<T>
	{
		using TYPE = typename T::Compiled;
	};

	template<class T>
	using Underlying = typename _Underlying<T>::TYPE;


	template<typename T> concept Compilable =
		requires (T t) {
		T::compile;
	};



	enum class SIG_TYPE:uint
	{
		Slot,
		RT,
		RTX,
		PSO,
		Layout
	};

		enum class SIG_TYPE_COMPILED:uint
	{
		Slot,
		RT,
		RTX,
		PSO,
		Layout
	};

	namespace SIG_TYPES
	{
		template<typename T> concept Slot =T::TYPE == SIG_TYPE::Slot;
	
	}
	

	namespace SIG_TYPES_COMPILED
	{
		template<typename T> concept Slot = T::TYPE == SIG_TYPE_COMPILED::Slot;
	
	}

}



