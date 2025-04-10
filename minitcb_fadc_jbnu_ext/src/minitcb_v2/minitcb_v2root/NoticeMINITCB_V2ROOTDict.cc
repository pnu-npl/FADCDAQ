// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME NoticeMINITCB_V2ROOTDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "ROOT/RConfig.hxx"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "NoticeMINITCB_V2ROOT.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_NKMINITCB_V2(void *p = nullptr);
   static void *newArray_NKMINITCB_V2(Long_t size, void *p);
   static void delete_NKMINITCB_V2(void *p);
   static void deleteArray_NKMINITCB_V2(void *p);
   static void destruct_NKMINITCB_V2(void *p);
   static void streamer_NKMINITCB_V2(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::NKMINITCB_V2*)
   {
      ::NKMINITCB_V2 *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::NKMINITCB_V2 >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("NKMINITCB_V2", ::NKMINITCB_V2::Class_Version(), "NoticeMINITCB_V2ROOT.h", 11,
                  typeid(::NKMINITCB_V2), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::NKMINITCB_V2::Dictionary, isa_proxy, 18,
                  sizeof(::NKMINITCB_V2) );
      instance.SetNew(&new_NKMINITCB_V2);
      instance.SetNewArray(&newArray_NKMINITCB_V2);
      instance.SetDelete(&delete_NKMINITCB_V2);
      instance.SetDeleteArray(&deleteArray_NKMINITCB_V2);
      instance.SetDestructor(&destruct_NKMINITCB_V2);
      instance.SetStreamerFunc(&streamer_NKMINITCB_V2);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::NKMINITCB_V2*)
   {
      return GenerateInitInstanceLocal(static_cast<::NKMINITCB_V2*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::NKMINITCB_V2*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr NKMINITCB_V2::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *NKMINITCB_V2::Class_Name()
{
   return "NKMINITCB_V2";
}

//______________________________________________________________________________
const char *NKMINITCB_V2::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::NKMINITCB_V2*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int NKMINITCB_V2::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::NKMINITCB_V2*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *NKMINITCB_V2::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::NKMINITCB_V2*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *NKMINITCB_V2::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::NKMINITCB_V2*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void NKMINITCB_V2::Streamer(TBuffer &R__b)
{
   // Stream an object of class NKMINITCB_V2.

   TObject::Streamer(R__b);
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_NKMINITCB_V2(void *p) {
      return  p ? new(p) ::NKMINITCB_V2 : new ::NKMINITCB_V2;
   }
   static void *newArray_NKMINITCB_V2(Long_t nElements, void *p) {
      return p ? new(p) ::NKMINITCB_V2[nElements] : new ::NKMINITCB_V2[nElements];
   }
   // Wrapper around operator delete
   static void delete_NKMINITCB_V2(void *p) {
      delete (static_cast<::NKMINITCB_V2*>(p));
   }
   static void deleteArray_NKMINITCB_V2(void *p) {
      delete [] (static_cast<::NKMINITCB_V2*>(p));
   }
   static void destruct_NKMINITCB_V2(void *p) {
      typedef ::NKMINITCB_V2 current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_NKMINITCB_V2(TBuffer &buf, void *obj) {
      ((::NKMINITCB_V2*)obj)->::NKMINITCB_V2::Streamer(buf);
   }
} // end of namespace ROOT for class ::NKMINITCB_V2

namespace {
  void TriggerDictionaryInitialization_NoticeMINITCB_V2ROOTDict_Impl() {
    static const char* headers[] = {
"NoticeMINITCB_V2ROOT.h",
nullptr
    };
    static const char* includePaths[] = {
"/home/npl/FADCDAQ/minitcb_fadc_jbnu_ext/include",
"/home/npl/install/root_install/include/root",
"/home/npl/FADCDAQ/minitcb_fadc_jbnu_ext/src/minitcb_v2/minitcb_v2root/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "NoticeMINITCB_V2ROOTDict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$NoticeMINITCB_V2ROOT.h")))  NKMINITCB_V2;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "NoticeMINITCB_V2ROOTDict dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "NoticeMINITCB_V2ROOT.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"NKMINITCB_V2", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("NoticeMINITCB_V2ROOTDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_NoticeMINITCB_V2ROOTDict_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_NoticeMINITCB_V2ROOTDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_NoticeMINITCB_V2ROOTDict() {
  TriggerDictionaryInitialization_NoticeMINITCB_V2ROOTDict_Impl();
}
