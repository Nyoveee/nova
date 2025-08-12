#pragma once
#include <type_traits>

// Calling Macro for double parameters / Idea from reflection.h :) 
#define Macro_Double_1(Macro, a, b) Macro(a,b)
#define Macro_Double_2(Macro, a, b, c, d) Macro(a,b) Macro_Double_1(Macro,c,d)
#define Macro_Double_3(Macro, a, b, c, d, e, f) Macro(a,b) Macro_Double_2(Macro,c,d,e,f)
#define Macro_Double_4(Macro, a, b, c, d, e, f, g, h) Macro(a,b) Macro_Double_3(Macro,c,d,e,f,g,h)
#define Macro_Double_5(Macro, a, b, c, d, e, f, g, h,i,j) Macro(a,b) Macro_Double_4(Macro,c,d,e,f,g,h,i,j)
#define Macro_Double_6(Macro, a, b, c, d, e, f, g, h,i,j,k,l) Macro(a,b) Macro_Double_5(Macro,c,d,e,f,g,h,i,j,k,l)
#define Macro_Double_7(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n) Macro(a,b) Macro_Double_6(Macro,c,d,e,f,g,h,i,j,k,l,m,n)
#define Macro_Double_8(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p) Macro(a,b) Macro_Double_7(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define Macro_Double_9(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p,q,r) Macro(a,b) Macro_Double_8(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)
#define Macro_Double_10(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p,q,r,s,t) Macro(a,b) Macro_Double_9(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)
#define Macro_Double_11(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p,q,r,s,t,u,v) Macro(a,b) Macro_Double_10(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v)
#define Macro_Double_12(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x) Macro(a,b) Macro_Double_11(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x)
#define Macro_Double_13(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z) Macro(a,b) Macro_Double_12(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z)
#define Macro_Double_14(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb) Macro(a,b) Macro_Double_13(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb)
#define Macro_Double_15(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb,cc,dd) Macro(a,b) Macro_Double_14(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb,cc,dd)
#define Macro_Double_16(Macro, a, b, c, d, e, f, g, h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb,cc,dd,ee,ff) Macro(a,b) Macro_Double_15(Macro,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb,cc,dd,ee,ff)

#define Get_Macro_Double(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,Name,...) Name
#define Call_Macro_Double(Macro,...) Get_Macro_Double(__VA_ARGS__,Macro_Double_16,,Macro_Double_15,,Macro_Double_14,,Macro_Double_13,,Macro_Double_12,,Macro_Double_11,,Macro_Double_10,,Macro_Double_9,,Macro_Double_8,,Macro_Double_7,,Macro_Double_6,,Macro_Double_5,,Macro_Double_4,,Macro_Double_3,,Macro_Double_2,,Macro_Double_1)(Macro,__VA_ARGS__)

// Managed Struct
#define Declaration(Type, Name) Type Name;
#define ConstructorDefinition(Type, Name) Name{native.Name},
#define ListInitialization(Type, Name) Name,
#define ManagedStruct(ManagedType,NativeType,...) \
public value struct ManagedType { \
	ManagedType(NativeType native) : Call_Macro_Double(ConstructorDefinition, __VA_ARGS__) type { #ManagedType } {} \
	NativeType native() { return {Call_Macro_Double(ListInitialization,__VA_ARGS__)};} \
	Call_Macro_Double(Declaration,__VA_ARGS__) \
	System::String^ type; \
};

// ManagedComponent
public interface class IManagedComponent {
	void SetEntityID(System::UInt32 entityID);
};
#define PropertyDeclaration(Type, Name) \
property Type Name \
{ \
	Type get(); \
	void set(Type value); \
}
#define ManagedComponentDeclaration(ManagedComponentType, ...) \
public value class ManagedComponentType : IManagedComponent { \
public: \
	virtual void SetEntityID(System::UInt32 entityID) { this->entityID = entityID; }; \
	Call_Macro_Double(PropertyDeclaration, __VA_ARGS__) \
private: \
	System::UInt32 entityID; \
};
#define ManagedComponentDefinition(ManagedComponentType,NativeComponentType, ManagedStructType, Name) \
ScriptingAPI::ManagedStructType ScriptingAPI::ManagedComponentType::Name::get() \
{ \
	return ManagedStructType{ ScriptingAPI::Interface::findNativeComponent<NativeComponentType>(entityID)->Name }; \
} \
void ScriptingAPI::ManagedComponentType::Name::set(ManagedStructType value) \
{ \
	ScriptingAPI::Interface::findNativeComponent<NativeComponentType>(entityID)->Name = value.native(); \
} 

// Todo, create version of the managedComponentDefinition that handles non struct like float