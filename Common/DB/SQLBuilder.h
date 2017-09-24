#pragma once
#include "atlstr.h"

class CSQLBuilder
{
public:
	CSQLBuilder(void) : m_bKeyWordAdded(false), m_strSQL("") {};
	~CSQLBuilder(void) {} ;

private:

	bool		m_bKeyWordAdded;
	CStringA	m_strSQL;

private:


public:

	void Reset() { 	m_strSQL.Empty(); }
	const char * const GetSQL() { return m_strSQL.GetString(); }

	CSQLBuilder & AliasCol(const char * pszTableAliasName, const char * pszColName)
	{
		if (pszTableAliasName != NULL && strlen(pszTableAliasName) > 0)
			m_strSQL.AppendFormat(" [%s].[%s] ", pszTableAliasName, pszColName);
		else
			m_strSQL.AppendFormat(" [%s] ", pszColName);

		return *this;
	}

	CSQLBuilder & SELECT() { m_strSQL.Append(" SELECT "); return *this; }
	CSQLBuilder & COUNT() { m_strSQL.Append( " COUNT(*) "); return *this; }
	CSQLBuilder & DISTINCT() { m_strSQL.Append(" DISTINCT "); return *this; }
	CSQLBuilder & MAX(const char * pszColName) { m_strSQL.AppendFormat(" MAX(%s) ", pszColName); return *this; }
	CSQLBuilder & WHERE() {	m_strSQL.Append(" WHERE "); return *this; }
	CSQLBuilder & AND() { m_strSQL.Append(" AND "); return *this; }
	CSQLBuilder & OR() { m_strSQL.Append(" OR "); return *this; }
	CSQLBuilder & LB() { m_strSQL.Append(" ( "); return *this; }
	CSQLBuilder & RB() { m_strSQL.Append(" ) "); return *this; }
	CSQLBuilder & SB() { m_strSQL.Append(" , "); return *this; }
	CSQLBuilder & ORDERBY() { m_strSQL.Append(" ORDER BY "); return *this; }
	CSQLBuilder & ASC() { m_strSQL.Append(" ASC "); return *this; }
	CSQLBuilder & DESC() { m_strSQL.Append(" DESC "); return *this; }
	CSQLBuilder & LIMIT(const long lLimit) { m_strSQL.AppendFormat(" LIMIT %d ", lLimit); return *this; }
    CSQLBuilder & OFFSET(const long lzbOffset) { m_strSQL.AppendFormat(" OFFSET %d ", max(0, lzbOffset)); return *this; }
    CSQLBuilder & VALUE(const char * pszValue) { m_strSQL.AppendFormat(" %s ", pszValue); return *this; }
    CSQLBuilder & VALUE(const __int64 i64Value) { m_strSQL.AppendFormat(" %I64d ", i64Value); return *this; }
    CSQLBuilder & VALUES() { m_strSQL.Append(" VALUES "); return *this; }
    CSQLBuilder & Append(const char * pszSTR) { m_strSQL.Append(pszSTR); return *this; }

	CSQLBuilder & FROM(const char * pszTableName, const char * pszTableAliasName = NULL)
	{
		if (pszTableAliasName != NULL && strlen(pszTableAliasName) > 0)
		{
			m_strSQL.AppendFormat(" FROM [%s] AS [%s] ", pszTableName, pszTableAliasName);
		}
		else
		{
			m_strSQL.AppendFormat(" FROM [%s] ", pszTableName);
		}
		return *this;
	}

	CSQLBuilder & LEFTJOINON(const char * pszTableName, const char * pszTableAliasName = NULL)
	{
		if (pszTableAliasName != NULL && strlen(pszTableAliasName) > 0)
		{
			m_strSQL.AppendFormat(" LEFT JOIN [%s] AS [%s] ON ", pszTableName, pszTableAliasName);
		}
		else
		{
			m_strSQL.AppendFormat(" LEFT JOIN [%s] ON ", pszTableName);
		} 
		return *this;
	}

	CSQLBuilder & INNERJOIN(const char * pszTableName, const char * pszTableAliasName = NULL)
	{
		if (pszTableAliasName != NULL && strlen(pszTableAliasName) > 0)
		{
			m_strSQL.AppendFormat(" INNER JOIN [%s] AS [%s] ON ", pszTableName, pszTableAliasName);
		}
		else
		{
			m_strSQL.AppendFormat(" INNER JOIN [%s] ON ", pszTableName);
		} 
		return *this;
	}

    CSQLBuilder & INSERTINTO(const char * pszTableName)
    {
        if (m_strSQL.GetLength() > 0) m_strSQL.Append(" ; ");
        m_strSQL.AppendFormat(" INSERT INTO [%s] ", pszTableName);
        return *this;
    }

	CSQLBuilder & INSERTINTO(const char * pszTableName, const long lColCount)
	{
		if (m_strSQL.GetLength() > 0) m_strSQL.Append(" ; ");
		m_strSQL.AppendFormat(" INSERT INTO [%s] VALUES( ", pszTableName);
		for (long l = 0; l < lColCount-1; l++ ) m_strSQL.Append("?, ");
		m_strSQL.Append("? )");
		return *this;
	}

	CSQLBuilder & DELETEFROM(const char * pszTableName)
	{
		if (m_strSQL.GetLength() > 0) m_strSQL.Append(" ; ");
		m_strSQL.AppendFormat(" DELETE FROM [%s] ", pszTableName); 
		return *this;
	}

	CSQLBuilder & UPDATESET(const char * pszTableName)
	{
		if (m_strSQL.GetLength() > 0) m_strSQL.Append(" ; ");
		m_strSQL.AppendFormat(" UPDATE [%s] SET ", pszTableName);
		return *this;
	}

	CSQLBuilder & ISNOTNULL(const char * pszColName, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.Append(" IS NOT NULL ");
		return *this;
	}

	CSQLBuilder & ISNULL(const char * pszColName, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.Append(" IS NULL ");
		return *this;
	}

	CSQLBuilder & In(const char * pszColName, const char * const pszSQL)
	{
		m_strSQL.AppendFormat(" [%s] IN ( %s ) ", pszColName, pszSQL);
		return *this;
	}

	CSQLBuilder & In(const char * pszColName, const long lCount, ...)
	{
		m_strSQL.AppendFormat(" [%s] IN ( ", pszColName);
		va_list vaList;
		va_start(vaList, lCount);
		for (long l = 0; l < lCount-1; l++) m_strSQL.AppendFormat("%d, ", va_arg(vaList, int));
		m_strSQL.AppendFormat("%d ) ", va_arg(vaList, int));
		va_end(vaList);		
		return *this;
	}

	CSQLBuilder & NOTIN(const char * pszColName, const char * const pszSQL)
	{
		m_strSQL.AppendFormat(" [%s] NOT IN ( %s ) ", pszColName, pszSQL);
		return *this;
	}

	CSQLBuilder & EQ(const char * pszColNameLeft, const char * pszColNameRight, const char * pszTableAliasNameLeft = NULL, const char * pszTableAliasNameRight = NULL)
	{
		AliasCol(pszTableAliasNameLeft, pszColNameLeft);
		m_strSQL.Append(" = ");
		AliasCol(pszTableAliasNameRight, pszColNameRight);
		return *this;
	}

	CSQLBuilder & EQ(const char * pszColName, const __int64 i64Value, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" = %I64d ", i64Value);
		return *this;
	}

	CSQLBuilder & EQSTR(const char * pszColName, const char * pszValue, bool bRawString = false, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		if (bRawString) m_strSQL.AppendFormat(" = %s ", pszValue); else m_strSQL.AppendFormat(" = '%s' ", pszValue);
		return *this;
	}

	CSQLBuilder & NOTEQSTR(const char * pszColName, const char * pszValue, bool bRawString = false, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		if (bRawString) m_strSQL.AppendFormat(" <> %s ", pszValue); else m_strSQL.AppendFormat(" <> '%s' ", pszValue);
		return *this;
	}

	CSQLBuilder & NOTEQ(const char * pszColName, const __int64 i64Value, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" <> %I64d ", i64Value);
		return *this;
	}

	CSQLBuilder & GT(const char * pszColName, const __int64 i64Value, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" > %I64d ", i64Value);
		return *this;
	}

	CSQLBuilder & GE(const char * pszColName, const __int64 i64Value, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" >= %I64d ", i64Value);
		return *this;
	}

	CSQLBuilder & LT(const char * pszColName, const __int64 i64Value, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" < %I64d ", i64Value);
		return *this;
	}
	CSQLBuilder & LE(const char * pszColName, const __int64 i64Value, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" <= %I64d ", i64Value);
		return *this;
	}

	CSQLBuilder & BETWEEN(const char * pszColName, const char * pszTableAliasName, const __int64 i64ValueLess, const __int64 i64ValueGreater)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat("BETWEEN %I64d AND %I64d ", i64ValueLess, i64ValueGreater);
		return *this;
	}

	CSQLBuilder & LIKEPrefix(const char * pszColName, const char * pszValue, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" LIKE '%s%%' ", pszValue);
		return *this;
	}

	CSQLBuilder & LIKESuffix(const char * pszColName, const char * pszValue, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" LIKE '%%%s' ", pszValue);
		return *this;
	}

	CSQLBuilder & LIKEAll(const char * pszColName, const char * pszValue, const char * pszTableAliasName = NULL)
	{
		AliasCol(pszTableAliasName, pszColName);
		m_strSQL.AppendFormat(" LIKE '%%%s%%' ", pszValue);
		return *this;
	}
	
};
