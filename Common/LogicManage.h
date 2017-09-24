#pragma once
#include <map>

template<class T>
class CLogicManage
{

public:
    CLogicManage(void) : m_pMapLogic(NULL) {};
    ~CLogicManage(void)
    {
        if ( m_pMapLogic != NULL )
        {
            std::map<const char *, void *, cmp_str>::iterator it = m_pMapLogic->begin();
            for ( ; it != m_pMapLogic->end(); it++ ) delete (T * const)it->second;
            m_pMapLogic->clear();
            std::map<const char *, void *, cmp_str>().swap(*m_pMapLogic);
            delete m_pMapLogic;
        }
    }

	struct cmp_str
	{
		bool operator()(const char *a, const char *b) const
		{
			return strcmp(a, b) < 0;
		}
	};

private:

	std::map<const char * , void *, cmp_str> * m_pMapLogic;

public:
    T * RegistLogic(char * const pLogicName, T * const pLogic)
    {
        if ( m_pMapLogic == NULL )
        {
            m_pMapLogic = new std::map<const char *, void *, cmp_str>;
        }

        std::map<const char *, void *, cmp_str>::iterator it = m_pMapLogic->find(pLogicName);
        if ( it != m_pMapLogic->end() )
        {
            delete (T * const)it->second;
        }
        (*m_pMapLogic)[pLogicName] = pLogic;
        return pLogic;
    }

    T * const GetLogicByName(const char * const pszLogicName)
    {
        _ASSERT(m_pMapLogic != NULL);
        std::map<const char *, void *, cmp_str>::iterator it = m_pMapLogic->find(pszLogicName);
        if ( it != m_pMapLogic->end() )
        {
            return it->second;
        }
        return NULL;
    }
};


