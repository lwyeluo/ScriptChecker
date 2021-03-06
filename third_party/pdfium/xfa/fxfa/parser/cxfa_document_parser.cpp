// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_document_parser.h"

#include "core/fxcrt/xml/cfx_xmldoc.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CXFA_DocumentParser::CXFA_DocumentParser(CXFA_FFNotify* pNotify)
    : m_pNotify(pNotify) {}

CXFA_DocumentParser::~CXFA_DocumentParser() {
  m_pDocument->ReleaseXMLNodesIfNeeded();
}

int32_t CXFA_DocumentParser::StartParse(
    const RetainPtr<IFX_SeekableStream>& pStream,
    XFA_PacketType ePacketID) {
  m_pDocument.reset();
  m_nodeParser.CloseParser();

  int32_t nRetStatus = m_nodeParser.StartParse(pStream, ePacketID);
  if (nRetStatus == XFA_PARSESTATUS_Ready) {
    m_pDocument = pdfium::MakeUnique<CXFA_Document>(GetNotify());
    m_nodeParser.SetFactory(m_pDocument.get());
  }
  return nRetStatus;
}

int32_t CXFA_DocumentParser::DoParse() {
  int32_t nRetStatus = m_nodeParser.DoParse();
  if (nRetStatus >= XFA_PARSESTATUS_Done) {
    ASSERT(m_pDocument);
    m_pDocument->SetRoot(m_nodeParser.GetRootNode());
  }
  return nRetStatus;
}

CXFA_FFNotify* CXFA_DocumentParser::GetNotify() const {
  return m_pNotify.Get();
}

CXFA_Document* CXFA_DocumentParser::GetDocument() const {
  return m_pDocument.get();
}
