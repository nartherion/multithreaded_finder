#include "parser.hpp"
#include "thread_safe_puts.hpp"

#include <Windows.h>

namespace finder::parser
{
    namespace 
    {
    
    bool CanAccessFolder(LPCTSTR folderName, DWORD genericAccessRights)
    {
	    bool bRet = false;
	    DWORD attributes = GetFileAttributes(folderName);
	    if ((attributes & FILE_ATTRIBUTE_HIDDEN) || (attributes & FILE_ATTRIBUTE_SYSTEM))
	    	return bRet;
	    DWORD length = 0;
	    if (!::GetFileSecurity(folderName, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
	    	| DACL_SECURITY_INFORMATION, NULL, NULL, &length) &&
	    	ERROR_INSUFFICIENT_BUFFER == ::GetLastError()) {
	    	PSECURITY_DESCRIPTOR security = static_cast<PSECURITY_DESCRIPTOR>(::malloc(length));
	    	if (security && ::GetFileSecurity(folderName, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
	    		| DACL_SECURITY_INFORMATION, security, length, &length)) {
	    		HANDLE hToken = NULL;
	    		if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY |
	    			TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken)) {
	    			HANDLE hImpersonatedToken = NULL;
	    			if (::DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken)) {
	    				GENERIC_MAPPING mapping = { 0xFFFFFFFF };
	    				PRIVILEGE_SET privileges = { 0 };
	    				DWORD grantedAccess = 0, privilegesLength = sizeof(privileges);
	    				BOOL result = FALSE;
	    				mapping.GenericRead = FILE_GENERIC_READ;
	    				mapping.GenericWrite = FILE_GENERIC_WRITE;
	    				mapping.GenericExecute = FILE_GENERIC_EXECUTE;
	    				mapping.GenericAll = FILE_ALL_ACCESS;
	    				::MapGenericMask(&genericAccessRights, &mapping);
	    				if (::AccessCheck(security, hImpersonatedToken, genericAccessRights,
	    					&mapping, &privileges, &privilegesLength, &grantedAccess, &result)) {
	    					bRet = (result == TRUE);
	    				}
	    				::CloseHandle(hImpersonatedToken);
	    			}
	    			::CloseHandle(hToken);
	    		}
	    		::free(security);
	    	}
	    }
	    return bRet;
    }

    void search_file_impl(std::filesystem::directory_entry current, std::string_view filename, 
        thread_utils::thread_pool& pool)
    {
        if (pool.is_work_done())
        {
            thread_utils::thread_safe_puts("[INFO] thread #", std::this_thread::get_id(), 
                " : stopped doing task, as the work is done", '\n');
            return;
        }
        thread_utils::thread_safe_puts("[INFO] thread #", std::this_thread::get_id(), " : just entered directory : ", 
            current.path(), '\n');
        std::filesystem::directory_iterator directory{ current };
        for (const auto& entry : directory)
        {
            if (entry.is_directory())
            {
                std::wstring p{ entry.path() };
                if (CanAccessFolder((LPCTSTR)p.c_str(), GENERIC_READ))
                    pool.push(std::bind(search_file_impl, entry, filename, std::ref(pool)));
                continue;
            }
            if (entry.is_regular_file() && entry.path().filename() == filename)
            {
                pool.set_work_done();
                thread_utils::thread_safe_puts("[SUCCESS] thread #", std::this_thread::get_id(), 
                    " : file has been found at path : ", entry.path(), '\n');
            }
        }
    }

    } // End of anonymous namespace

    void search_file(std::filesystem::directory_entry initial, std::string_view filename, size_t parallel)
    {
        thread_utils::thread_pool workers{ parallel };
        workers.push(std::bind(search_file_impl, initial, filename, std::ref(workers)));
        workers.start();
        while (workers.is_work_done())
            continue;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

}