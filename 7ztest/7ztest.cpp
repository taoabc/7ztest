// 7ztest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "7zFileStream.h"
#include "ArchiveOpenCallBack.h"
#include "ArchiveExtractCallBack.h"

#include "ult\library.h"
#include "ult\file-dir.h"
#include "ult\string-operate.h"
#include "ult\file-io.h"

#include <iostream>
#include <string>
#include <windows.h>

DEFINE_GUID(CLSID_CFormat7z,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

const wchar_t* kDllName = L"7zxa.dll";

typedef UINT32 (__stdcall *CreateObjectProc)(const GUID*, const GUID*, void**);

int _tmain(int argc, _TCHAR* argv[])
{
  ult::Library lib;
  lib.Load(kDllName);
  if (!lib.IsLoaded()) {
    printf("load moudle failed");
    return 1;
  }
  CreateObjectProc CreateObject = (CreateObjectProc)lib.GetProc("CreateObject");
  if (NULL == CreateObject) {
    return 1;
  }

  CMyComPtr<IInArchive> archive;
  if (S_OK != CreateObject(&CLSID_CFormat7z, &IID_IInArchive, (void**)&archive)) {
    printf("can not get class object");
    return 1;
  }

  InFileStream* file_spec = new InFileStream;
  CMyComPtr<IInStream> file(file_spec);

  if (!file_spec->Open(L"E:\\temp\\test.7z")) {
    printf("can not open archive file");
    return 1;
  }

  {
    ArchiveOpenCallBack* open_callback_spec = new ArchiveOpenCallBack;
    CMyComPtr<IArchiveOpenCallback> open_callback(open_callback_spec);
    open_callback_spec->password_is_defined_ = false;

    if (archive->Open(file, 0, open_callback) != S_OK) {
      printf("can not open archive");
      return 1;
    }
  }

  ArchiveExtractCallBack* extract_callback_spec = new ArchiveExtractCallBack;
  CMyComPtr<IArchiveExtractCallback> extract_callback(extract_callback_spec);
  extract_callback_spec->Init(archive, L"c:\\windows\\asdfasdf");

  HRESULT result = archive->Extract(NULL, (UInt32)(Int32)-1, false, extract_callback);
  if (result != S_OK) {
    printf("extract error");
    return 1;
  }

	return 0;
}