#include "StdAfx.h"
#include "ArchiveExtractCallBack.h"

static HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool* result) {
  PROPVARIANT prop;
  RINOK(archive->GetProperty(index, propID, &prop));
  if (prop.vt == VT_BOOL)
    *result = (prop.boolVal != VARIANT_FALSE);
  else if (prop.vt == VT_EMPTY)
    *result = false;
  else
    return E_FAIL;
  return S_OK;
}

static HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool* result)
{
  return IsArchiveItemProp(archive, index, kpidIsDir, result);
}

const wchar_t* ArchiveExtractCallBack::kEmptyFileAlias_ = L"[Content]";

ArchiveExtractCallBack::ArchiveExtractCallBack(void) :
    password_is_defined_(false) {
}

STDMETHODIMP ArchiveExtractCallBack::SetTotal(UInt64 size) {
  return S_OK;
}

STDMETHODIMP ArchiveExtractCallBack::SetCompleted(const UInt64* complete_value) {
  return S_OK;
}

STDMETHODIMP ArchiveExtractCallBack::GetStream(UInt32 index,
                                               ISequentialOutStream **out_stream,
                                               Int32 ask_extract_mode) {
  *out_stream = 0;
  out_filestream_->Release();

  PROPVARIANT prop;
  RINOK(archive_handler_->GetProperty(index, kpidPath, &prop) != 0);
  {
    //Get name
    std::wstring full_path;
    if (prop.vt == VT_EMPTY) {
      full_path = kEmptyFileAlias_;
    } else {
      if (prop.vt != VT_BSTR) {
        return E_FAIL;
      }
      full_path = prop.bstrVal;
    }
    file_path_ = full_path;
  }

  if (ask_extract_mode != NArchive::NExtract::NAskMode::kExtract)
    return S_OK;

  {
    //Get attrib
    PROPVARIANT prop;
    RINOK(archive_handler_->GetProperty(index, kpidAttrib, &prop));
    if (prop.vt == VT_EMPTY) {
      processed_fileinfo_.attrib = 0;
      processed_fileinfo_.attrib_defined = false;
    } else {
      if (prop.vt != VT_UI4) {
        return E_FAIL;
      }
      processed_fileinfo_.attrib = prop.ulVal;
      processed_fileinfo_.attrib_defined = true;
    }
  }

  RINOK(IsArchiveItemFolder(archive_handler_, index, &processed_fileinfo_.isdir));
  
}

STDMETHODIMP ArchiveExtractCallBack::PrepareOperation(Int32 ask_extrack_mode) {
  return S_OK;
}

STDMETHODIMP ArchiveExtractCallBack::SetOperationResult(Int32 operation_result) {
  return S_OK;
}

STDMETHODIMP ArchiveExtractCallBack::CryptoGetTextPassword(BSTR* password) {
  if (!password_is_defined_) {
    return E_ABORT;
  }
  return StringToBstr(password_.c_str(), password);
}
