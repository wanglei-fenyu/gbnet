#pragma once

#include <stdexcept>

template <typename T>
class Singleton {
public:
    template<typename... Args>
    static T* Instance( Args&&... args ) {
        if ( m_pInstance == nullptr )
            m_pInstance = new T( std::forward<Args>( args )... );
        return m_pInstance;
    }

    static void DestroyInstance( ) {
        delete m_pInstance;
        m_pInstance = nullptr;
    }

private:
    inline static T* m_pInstance;
};
