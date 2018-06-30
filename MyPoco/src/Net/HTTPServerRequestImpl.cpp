//
// HTTPServerRequestImpl.cpp
//
// Library: Net
// Package: HTTPServer
// Module:  HTTPServerRequestImpl
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#include "Net/HTTPServerRequestImpl.h"
#include "Net/HTTPServerResponseImpl.h"
#include "Net/HTTPServerSession.h"
#include "Net/HTTPHeaderStream.h"
#include "Net/HTTPStream.h"
#include "Net/HTTPFixedLengthStream.h"
#include "Net/HTTPChunkedStream.h"
#include "Net/HTTPServerParams.h"
#include "Net/StreamSocket.h"
#include "Poco/String.h"


using Poco::icompare;


namespace Poco {
namespace Net {


HTTPServerRequestImpl::HTTPServerRequestImpl(HTTPServerResponseImpl& response, HTTPServerSession& session, HTTPServerParams* pParams):
	_response(response),
	_session(session),
	_pStream(0),
	_pParams(pParams, true)
{
	response.attachRequest(this);

	HTTPHeaderInputStream hs(session);
	read(hs);
	
	// Now that we know socket is still connected, obtain addresses
	_clientAddress = session.clientAddress();
	_serverAddress = session.serverAddress();
	
	if (getChunkedTransferEncoding())
		_pStream = new HTTPChunkedInputStream(session);
	else if (hasContentLength())
#if defined(POCO_HAVE_INT64)
		_pStream = new HTTPFixedLengthInputStream(session, getContentLength64());
#else
		_pStream = new HTTPFixedLengthInputStream(session, getContentLength());
#endif
	else if (getMethod() == HTTPRequest::HTTP_GET || getMethod() == HTTPRequest::HTTP_HEAD || getMethod() == HTTPRequest::HTTP_DELETE)
		_pStream = new HTTPFixedLengthInputStream(session, 0);
	else
		_pStream = new HTTPInputStream(session);
}


HTTPServerRequestImpl::~HTTPServerRequestImpl()
{
	delete _pStream;
}


bool HTTPServerRequestImpl::secure() const
{
	return _session.socket().secure();
}


StreamSocket& HTTPServerRequestImpl::socket()
{
	return _session.socket();
}


StreamSocket HTTPServerRequestImpl::detachSocket()
{
	return _session.detachSocket();
}


} } // namespace Poco::Net
