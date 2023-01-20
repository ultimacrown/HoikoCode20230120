#pragma once

template <class T>
ref class RefGenerics {
public:
	T *obj;
	RefGenerics(T *t) : obj(t) {}
};

template <class T>
value class ValueGenerics {
public:
	T *obj;
	ValueGenerics(T *t) : obj(t) {}
};

template <class T>
ref class SmartPtr {
public:
	T *obj;
	SmartPtr() {
		this->obj = new T();
	}
	~SmartPtr() {
		delete this->obj;
		this->obj = nullptr;
		//System::Windows::Forms::MessageBox::Show(L"destruct");
	}
	!SmartPtr() {
		delete this->obj;
		this->obj = nullptr;
		//System::Windows::Forms::MessageBox::Show(L"finalize");
	}
};