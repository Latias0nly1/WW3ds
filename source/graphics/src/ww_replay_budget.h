#pragma once
#include <cstddef>
namespace t2p 
{
    template < class Cache > 
    inline std::size_t trimReplayBudget(Cache& cache,
const std::size_t& live,
    std::size_t budget,
std::size_t slack)
    {
        if(live <= budget || live - budget <= slack)return 0;
        std::size_t removed = 0;
        while(!cache.empty() 
&& live > budget)
        {
            cache.erase(cache.begin());
            ++removed;
            
        }
        return removed;
        
    }
    
}
