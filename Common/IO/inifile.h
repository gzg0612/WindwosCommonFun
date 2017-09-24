//// $_FILEHEADER_BEGIN **********************************************
// 北京新浪信息技术有限公司版权所有
// Copyright (C) Sina Corporation.  All Rights Reserved
// 文件名称：IniFile.h
// 创建日期：2010.11.16
// 创建人：刘亮亮
// 文件说明：INI 文件简单封装
// $_FILEHEADER_END **********************************************

#pragma once


class CIniFile
{
public:
	void SetFileName(wstring afile)
	{
		mstrfile = afile;
	}

	void SetSession(wstring astrSession)
	{
		mstrSession = astrSession;
	}


	//////////////////////////////////////////////////////////////////////////

	int GetValueInt(wstring astrKey, int aDefault)
	{
		_ASSERT( !mstrfile.empty() );
		
		return ::GetPrivateProfileInt(mstrSession.c_str(), astrKey.c_str(), aDefault, mstrfile.c_str());
	}
	
	void GetValueString(wstring astrKey, wstring astrDefault, OUT wstring &aRet)
	{
		_ASSERT( !mstrfile.empty() );

		TCHAR	lszValue[4096]=_T("");
		DWORD	ldwSize=0;

		ldwSize = ::GetPrivateProfileString( mstrSession.c_str(),astrKey.c_str(), astrDefault.c_str(), lszValue, sizeof(lszValue)/sizeof(TCHAR) ,mstrfile.c_str());
		lszValue[ldwSize] = _T('\0');
		aRet = lszValue;
	}
	
	
	//////////////////////////////////////////////////////////////////////////
	bool WriteValueInt(wstring astrKey, int aiValue)
	{
		_ASSERT( !mstrfile.empty() );

		TCHAR	lszvalue[1024]=_T("");	
		_itow_s(aiValue, lszvalue, 10);

		return (TRUE==::WritePrivateProfileString(mstrSession.c_str(), astrKey.c_str(), lszvalue, mstrfile.c_str()));
	}

	bool WriteValueString(wstring astrKey, wstring astrValue)
	{
		_ASSERT( !mstrfile.empty() );

		return (TRUE==::WritePrivateProfileString(mstrSession.c_str(), astrKey.c_str(), astrValue.c_str(), mstrfile.c_str()));
	}

private:
	wstring mstrfile;
	wstring mstrSession;
};