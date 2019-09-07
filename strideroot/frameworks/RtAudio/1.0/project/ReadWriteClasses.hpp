#ifndef READWRITECLASSES_HPP
#define READWRITECLASSES_HPP

#include <mutex>

#include "Synchronization.hpp"

namespace stride {

/////////////////////////////////////////////////////////////////////
// CLASS NAMING SCHEME AND INTERFACES
/////////////////////////////////////////////////////////////////////

/* --
      SD        Single Domain
      MD        Multi Domain
      R         Read
      W         Write
      Rst       Reset
-- */


//    To Read:
//        Data.Swap();
//        Object.SomeProcess(Data.Read());

//    To Write (see note):
//        if (Data.Lock()) {
//            Object.SomeProcess(Data.Write());
//            Data.Unlock();
//        }
//        else {
//            DataType Temp;
//            Object.SomeProcess(Temp);
//        }

//    To Reset:
//        Data.Reset();

// The else in the write is required only if:
//    + the data lock is a try-lock and
//    + "SomeProcess" writes to other arguments too.


/////////////////////////////////////////////////////////////////////
// SIGNAL INTERFACE
/////////////////////////////////////////////////////////////////////

template<class DataType>
class SignalReadWriteResetInterface {
public:
    virtual void Swap(void) = 0;
    virtual DataType Read(void) = 0;
    virtual bool Lock(void) = 0;
    virtual DataType * Write(void) = 0;
    virtual void Unlock(void) = 0;
};

/////////////////////////////////////////////////////////////////////
// BUNDLE INTERFACE
/////////////////////////////////////////////////////////////////////

template<class DataType>
class BundleReadWriteResetInterface {
public:
    virtual void Swap(void) = 0;
    virtual DataType * Read(void) = 0;
    virtual bool Lock(void) = 0;
    virtual DataType * Write(void) = 0;
    virtual void Unlock(void) = 0;
};

/////////////////////////////////////////////////////////////////////
// SIGNALS
/////////////////////////////////////////////////////////////////////

// Single domain read and write with no reset
template<class ClassType, class DataType>
class Signal_SDRW : public SignalReadWriteResetInterface<DataType> {
public:
    Signal_SDRW(void (ClassType::*Init)(DataType *), ClassType *object) {
        (object->*Init)(&m_Signal);
    }

    void Swap(void) {
    }

    DataType Read(void) {
        return m_Signal;
    }

    bool Lock(void) {
        return true;
    }

    DataType* Write(void) {
        return &m_Signal;
    }

    void Unlock(void) {
    }

private:
    DataType m_Signal;
};

// Single domain read, write and reset
template<class ClassType, class DataType>
class Signal_SDRWRst : public SignalReadWriteResetInterface<DataType> {
public:
    Signal_SDRWRst(void (ClassType::*Init)(DataType *), ClassType *object) {
        (object->*Init)(&m_Signal_Default);
        m_Signal = m_Signal_Default;
    }

    void Swap(void) {
    }

    DataType Read(void) {
        return m_Signal;
    }

    bool Lock(void) {
        return true;
    }

    DataType* Write(void) {
        return &m_Signal;
    }

    void Unlock(void) {
    }

    void Reset(void) {
        m_Signal = m_Signal_Default;
    }

private:
    DataType m_Signal_Default;
    DataType m_Signal;
};

// Single domain read and write with multi domain reset
// A synchronization scheme has to be defined between the read/write domain and ALL reset domains.
// Individual definitons are not enough, unless they are identical.
template<class ClassType, class DataType, class ResetReadLockType, class ResetWriteLockType>
class Signal_SDRW_MDRst : public SignalReadWriteResetInterface<DataType> {
public:
    Signal_SDRW_MDRst(void (ClassType::*Init)(DataType *), ClassType *object, std::mutex *resetMutex) : m_ResetMutex(resetMutex) {
        (object->*Init)(&m_Signal_Default);
        m_Signal = m_Signal_Default;
    }

    void Swap(void) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            if (m_Reset_Invoked) {
                m_Reset_Invoked = false;
                m_Signal = m_Signal_Default;
            }
        }
    }

    DataType Read(void) {
        return m_Signal;
    }

    bool Lock(void) {
        return true;
    }

    DataType* Write(void) {
        // TODO: Review the synchronization and if statement. It should stay as is.
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            m_Reset_Invoked = false;
        }
        return &m_Signal;
    }

    void Unlock(void) {
    }

    void Reset(void) {
        sync::Synchronization<ResetWriteLockType> ResetSync(m_ResetMutex, ResetWriteLockType(), sync::scoped());
        if (ResetSync(ResetWriteLockType())) {
            m_Reset_Invoked = true;
        }
    }

private:
    std::mutex *m_ResetMutex;

    DataType m_Signal;
    DataType m_Signal_Default;
    bool m_Reset_Invoked = false;
};

// Singal domain read with multi domain write and no reset
// A synchronization scheme has to be defined between the read domain and ALL write domains.
// Individual definitons are not enough, unless they are identical.
template<class ClassType, class DataType, class DataReadLockType, class DataWriteLockType>
class Signal_SDR_MDW : public SignalReadWriteResetInterface<DataType> {
public:
    Signal_SDR_MDW(void (ClassType::*Init)(DataType *), ClassType *object, std::mutex *dataMutex) : m_DataMutex(dataMutex) {
        (object->*Init)(&m_Signal_Read);
        m_Signal_Write = m_Signal_Read;
    }

    void Swap(void) {
        sync::Synchronization<DataReadLockType> DataSync(m_DataMutex, DataReadLockType(), sync::scoped());
        if (DataSync(DataReadLockType())) {
            m_Signal_Read = m_Signal_Write;
        }
    }

    DataType Read(void) {
        return m_Signal_Read;
    }

    bool Lock(void) {
        sync::Synchronization<DataWriteLockType> DataSync(m_DataMutex, DataWriteLockType(), sync::unscoped());
        return DataSync(DataWriteLockType());
    }

    DataType* Write(void) {
        return &m_Signal_Write;
    }

    void Unlock(void) {
        m_DataMutex->unlock();
    }

private:
    std::mutex *m_DataMutex;

    DataType m_Signal_Read;
    DataType m_Signal_Write;
};

// TODO: Review the synchronization
// Single domian read with mutli domain write and multi domain reset
// A synchronization scheme has to be defined between the read domain and ALL write domains. This controls the DataRead/DataWrite locks.
// A synchronization scheme has to be defined between the read domain and ALL reset domains. This controls the ResetWrite locks.
// A synchronization scheme has to be defined between the read domain and ALL domains. This controls the StateRead lock. StateWrite follows DataWrite and ResetWrite.
// IN THIS CASE ONLY A RESET MUTEX IS NOT REQUIRED!
// Individual definitons are not enough, unless they are identical.
template<class ClassType, class DataType, class DataReadLockType, class DataWriteLockType, class ResetWriteLockType, class StateReadLockType>
class Signal_SDR_MDWRst : public SignalReadWriteResetInterface<DataType> {
public:
    Signal_SDR_MDWRst(void (ClassType::*Init)(DataType *), ClassType *object, std::mutex *dataMutex) : m_DataMutex (dataMutex) {
        (object->*Init)(&m_Signal_Default);
        m_Signal_Read = m_Signal_Default;
    }

    void Swap(void) {
        sync::Synchronization<StateReadLockType> StateSync(&m_StateMutex, StateReadLockType(), sync::unscoped());
        if (StateSync(StateReadLockType())) {
            m_Swap_State = m_State;
            m_State = State::NONE;
            m_StateMutex.unlock();
        }
        switch(m_Swap_State) {
            case State::RESET: {
                m_Signal_Read = m_Signal_Default;
                break;
            }
            case State::WRITE: {
                sync::Synchronization<DataReadLockType> DataSync(m_DataMutex, DataReadLockType(), sync::scoped());
                if (DataSync(DataReadLockType())) {
                    m_Signal_Read = m_Signal_Write;
                };
                break;
            }
            case State::NONE: {
                break;
            }
        }
    }

    DataType Read(void) {
        return m_Signal_Read;
    }

    bool Lock(void) {
        sync::Synchronization<DataWriteLockType> DataSync(m_DataMutex, DataWriteLockType(), sync::unscoped());
        return DataSync(DataWriteLockType());
    }

    DataType* Write(void) {
        sync::Synchronization<DataWriteLockType> StateSync(&m_StateMutex, DataWriteLockType(), sync::scoped());
        if (StateSync(DataWriteLockType())) {
            m_State = State::WRITE;
        }
        return &m_Signal_Write;
    }

    void Unlock(void) {
        m_DataMutex->unlock();
    }

    void Reset(void) {
        sync::Synchronization<ResetWriteLockType> StateSync(&m_StateMutex, ResetWriteLockType(), sync::scoped());
            if (StateSync(ResetWriteLockType())) {
                m_State = State::RESET;
            }
        }

private:
    std::mutex *m_DataMutex;

    DataType m_Signal_Default;
    DataType m_Signal_Read;
    DataType m_Signal_Write;

    enum State {NONE = 1, WRITE = 2, RESET = 3 };
    State m_State = State::NONE;
    State m_Swap_State = State::NONE;
    std::mutex m_StateMutex;
};

/////////////////////////////////////////////////////////////////////
// BUNDLES
/////////////////////////////////////////////////////////////////////

template<class ClassType, class DataType>
class Bundle_SDRW : public BundleReadWriteResetInterface<DataType> {
public:
    Bundle_SDRW(void (ClassType::*Init) (DataType[]), ClassType *object, int size) {
        m_Bundle = new DataType[size];
        (object->*Init)(m_Bundle);
    }

    ~Bundle_SDRW(void) {
        delete[] m_Bundle;
    }

    void Swap(void) {
    }

    DataType * Read(void) {
        return m_Bundle;
    }

    bool Lock(void) {
        return true;
    }

    DataType * Write(void) {
        return m_Bundle;
    }

    void Unlock(void) {
    }

private:
    DataType *m_Bundle;
};

// Single domain read, write and reset
template<class ClassType, class DataType>
class Bundle_SDRWRst : public BundleReadWriteResetInterface<DataType> {
public:
    Bundle_SDRWRst(void (ClassType::*Init) (DataType[]), ClassType *object, int size) : m_Init(Init), m_Object(object), m_Size(size) {
        m_Bundle = new DataType[m_Size];
        (m_Object->*m_Init)(m_Bundle);
    }

    ~Bundle_SDRWRst(void) {
        delete[] m_Bundle;
    }

    void Swap(void) {
    }

    DataType * Read(void) {
        return m_Bundle;
    }

    bool Lock(void) {
        return true;
    }

    DataType * Write(void) {
        return m_Bundle;
    }

    void Unlock(void) {
    }

    void Reset(void) {
        (m_Object->*m_Init)(m_Bundle);
    }

private:
    void (ClassType::*m_Init)(DataType *);
    ClassType *m_Object;
    const int m_Size;

    DataType *m_Bundle;
};

// Single domain read and write with multi domain reset
// A synchronization scheme has to be defined between the read/write domain and ALL reset domains.
// Individual definitons are not enough, unless they are identical.
template<class ClassType, class DataType, class ResetReadLockType, class ResetWriteLockType>
class Bundle_SDRW_MDRst : public BundleReadWriteResetInterface<DataType> {
public:
    Bundle_SDRW_MDRst(void (ClassType::*Init) (DataType[]), ClassType *object, std::mutex *resetMutex, int size) : m_Init(Init), m_Object(object), m_ResetMutex(resetMutex), m_Size(size) {
        m_Bundle_A = new DataType[m_Size];
        m_Bundle_B = new DataType[m_Size];
        m_Bundle = m_Bundle_A;
        m_Bundle_Reset = m_Bundle_B;
        (m_Object->*m_Init)(m_Bundle);
    }

    ~Bundle_SDRW_MDRst(void) {
        delete[] m_Bundle_A;
        delete[] m_Bundle_B;
    }

    void Swap(void) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            if (m_Reset_Invoked) {
                m_Reset_Invoked = false;
                m_BufferSwap();
            }
        }
    }

    DataType * Read(void) {
        return m_Bundle;
    }

    bool Lock(void) {
        return true;
    }

    DataType * Write(void) {
        // TODO: Review the synchronization and if statement. It should stay as is.
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            m_Reset_Invoked = false;
        }
        return m_Bundle;
    }

    void Unlock(void) {
    }

    void Reset(void) {
        sync::Synchronization<ResetWriteLockType> ResetSync(m_ResetMutex, ResetWriteLockType(), sync::scoped());
        if (ResetSync(ResetWriteLockType())) {
            if (m_Reset_Invoked) return;
            m_Reset_Invoked = true;
            (m_Object->*m_Init)(m_Bundle_Reset);
        }
    }

private:
    void (ClassType::*m_Init)(DataType *);
    ClassType *m_Object;
    std::mutex *m_ResetMutex;
    const int m_Size;

    DataType *m_Bundle_A;
    DataType *m_Bundle_B;
    DataType *m_Bundle;
    DataType *m_Bundle_Reset;
    bool m_Reset_Invoked = false;

    void m_BufferSwap() {
        DataType *buffer = m_Bundle;
        m_Bundle = m_Bundle_Reset;
        m_Bundle_Reset = buffer;
    }
};

// Singal domain read with multi domain write and no reset
// A synchronization scheme has to be defined between the read domain and ALL write domains.
// Individual definitons are not enough, unless they are identical.
template<class ClassType, class DataType, class DataReadLockType, class DataWriteLockType>
class Bundle_SDR_MDW : public BundleReadWriteResetInterface<DataType> {
public:
    Bundle_SDR_MDW(void (ClassType::*Init) (DataType[]), ClassType *object, std::mutex *dataMutex, int size) : m_DataMutex(dataMutex), m_Size(size) {
        m_Bundle_A = new DataType[m_Size];
        m_Bundle_B = new DataType[m_Size];
        m_Bundle_Read = m_Bundle_A;
        m_Bundle_Write = m_Bundle_B;
        (object->*Init)(m_Bundle_Read);
    }

    ~Bundle_SDR_MDW(void) {
        delete[] m_Bundle_A;
        delete[] m_Bundle_B;
    }

    void Swap(void) {
        sync::Synchronization<DataReadLockType> DataSync(m_DataMutex, DataReadLockType(), sync::scoped());
        if (DataSync(DataReadLockType())) {
            if (m_Write_Invoked){
                m_Write_Invoked = false;
                m_BufferSwap();
            }
        }
    }

    DataType * Read(void) {
        return m_Bundle_Read;
    }

    bool Lock(void) {
        sync::Synchronization<DataWriteLockType> DataSync(m_DataMutex, DataWriteLockType(), sync::unscoped());
        return DataSync(DataWriteLockType());
    }

    DataType * Write(void) {
        m_Write_Invoked = true;
        return m_Bundle_Write;
    }

    void Unlock(void) {
        m_DataMutex->unlock();
    }

private:
    std::mutex *m_DataMutex;
    const int m_Size;

    DataType *m_Bundle_A;
    DataType *m_Bundle_B;
    DataType *m_Bundle_Read;
    DataType *m_Bundle_Write;
    bool m_Write_Invoked = false;

    void m_BufferSwap() {
        DataType *buffer = m_Bundle_Read;
        m_Bundle_Read = m_Bundle_Write;
        m_Bundle_Write = buffer;
    }
};

// TODO: Review the synchronization
// Single domian read with mutli domain write and multi domain reset
// A synchronization scheme has to be defined between the read domain and ALL write domains. This controls the DataRead/DataWrite locks.
// A synchronization scheme has to be defined between the read domain and ALL reset domains. This controls the ResetWrite locks.
// A synchronization scheme has to be defined between the read domain and ALL domains. This controls the StateRead lock. StateWrite follows DataWrite and ResetWrite.
// Individual definitons are not enough, unless they are identical.
template<class ClassType, class DataType, class DataReadLockType, class DataWriteLockType, class ResetReadLockType, class ResetWriteLockType, class StateReadLockType>
class Bundle_SDR_MDWRst : BundleReadWriteResetInterface<DataType> {
public:
    Bundle_SDR_MDWRst(void (ClassType::*Init) (DataType[]), ClassType *object, std::mutex *dataMutex, std::mutex *resetMutex, int size) : m_Init(Init), m_Object(object), m_DataMutex(dataMutex), m_ResetMutex(resetMutex), m_Size(size) {
        m_Bundle_A = new DataType[m_Size];
        m_Bundle_B = new DataType[m_Size];
        m_Bundle_C = new DataType[m_Size];
        m_Bundle_Read = m_Bundle_A;
        m_Bundle_Write = m_Bundle_B;
        m_Bundle_Reset = m_Bundle_C;
        (m_Object->*m_Init)(m_Bundle_Read);
    }

    ~Bundle_SDR_MDWRst(void) {
        delete[] m_Bundle_A;
        delete[] m_Bundle_B;
        delete[] m_Bundle_C;
    }

    void Swap(void) {
        sync::Synchronization<StateReadLockType> StateSync(&m_StateMutex, StateReadLockType(), sync::unscoped());
        if (StateSync(StateReadLockType())) {
            m_Swap_State = m_State;
            m_State = State::NONE;
            m_StateMutex.unlock();
        }
        switch(m_Swap_State) {
            case State::RESET: {
                sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
                if (ResetSync(ResetReadLockType())) {
                    m_BufferSwap(m_Bundle_Reset);
                }
                break;
            }
            case State::WRITE: {
                sync::Synchronization<DataReadLockType> DataSync(m_DataMutex, DataReadLockType(), sync::scoped());
                if (DataSync(DataReadLockType())) {
                    m_BufferSwap(m_Bundle_Write);
                };
                break;
            }
            case State::NONE: {
                break;
            }
        }
    }

    DataType * Read(void) {
        return m_Bundle_Read;
    }

    bool Lock(void) {
        sync::Synchronization<DataWriteLockType> DataSync(m_DataMutex, DataWriteLockType(), sync::unscoped());
        return DataSync(DataWriteLockType());
    }

    DataType * Write(void) {
        sync::Synchronization<DataWriteLockType> StateSync(&m_StateMutex, DataWriteLockType(), sync::scoped());
        if (StateSync(DataWriteLockType())) {
            m_State = State::WRITE;
        }
        return m_Bundle_Write;
    }

    void Unlock(void) {
        m_DataMutex->unlock();
    }

    void Reset(void) {
        sync::Synchronization<ResetWriteLockType> ResetSync(m_ResetMutex, ResetWriteLockType(), sync::scoped());
        if (ResetSync(ResetWriteLockType())) {
            sync::Synchronization<ResetWriteLockType> StateSync(&m_StateMutex, ResetWriteLockType(), sync::unscoped());
            if (StateSync(ResetWriteLockType())) {
                m_State = State::RESET;
                m_StateMutex.unlock();
            }
            (m_Object->*m_Init)(m_Bundle_Reset);
        }
    }

private:
    void (ClassType::*m_Init)(DataType *);
    ClassType *m_Object;
    std::mutex *m_DataMutex;
    std::mutex *m_ResetMutex;
    const int m_Size;

    DataType *m_Bundle_A;
    DataType *m_Bundle_B;
    DataType *m_Bundle_C;
    DataType *m_Bundle_Read;
    DataType *m_Bundle_Write;
    DataType *m_Bundle_Reset;

    enum State {NONE = 1, WRITE = 2, RESET = 3 };
    State m_State = State::NONE;
    State m_Swap_State = State::NONE;
    std::mutex m_StateMutex;

    void m_BufferSwap(DataType *Bundle) {
        DataType *buffer = m_Bundle_Read;
        m_Bundle_Read = Bundle;
        Bundle = buffer;
    }
};

} // END NAMESPACE STRIDE

#endif // READWRITECLASSES_HPP
