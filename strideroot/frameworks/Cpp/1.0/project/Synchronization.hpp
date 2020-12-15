#ifndef SYNCHRONIZATION_HPP
#define SYNCHRONIZATION_HPP

#include <mutex>

namespace stride {

namespace sync {

class scoped {};
class unscoped {};

class lock {};
class try_lock {};

template <class LockType>
class Synchronization {
public:
    // Unscoped Lock
    Synchronization(std::mutex *m, sync::lock, sync::unscoped) {
        m->lock();
    }

    // Unscoped TryLock
    Synchronization(std::mutex *m, sync::try_lock, sync::unscoped) {
        m_LockOwned = m->try_lock();
    }

    // Scoped Lock
    Synchronization(std::mutex *m, sync::lock, sync::scoped) : m_ScopedResetLock(*m) {
    }

    // Unscoped TryLock
    Synchronization(std::mutex *m, sync::try_lock, sync::scoped) : m_ScopedResetLock(*m, std::try_to_lock) {
        m_LockOwned = m_ScopedResetLock.owns_lock();
    }

    // Generic Lock Check
    bool operator()(LockType) {
        return true;
    }

private:
    std::unique_lock<std::mutex> m_ScopedResetLock;
    bool m_LockOwned;
};

// Specialized Lock Check for TryLock
template<> bool Synchronization<sync::try_lock>::operator()(sync::try_lock) {
    return m_LockOwned;
}

} // END NAMESPACE SYNC

} // END NAMESPACE STRIDE

#endif // SYNCHRONIZATION_HPP
