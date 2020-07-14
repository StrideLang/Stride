#ifndef BUFFERCLASSES_HPP
#define BUFFERCLASSES_HPP

#include <mutex>

#include "Synchronization.hpp"

namespace stride {

template <class DataType>
class Buffer {
public:
    virtual void Read(DataType *) = 0;
    virtual void Write(DataType) = 0;
    virtual void Reset(void) {}
};

// ==================================================================
// SINGLE PROCESS WRITE SIGNAL THEN READ
// ==================================================================

template <class DataType>
class Buffer_WriteSignal_ReadSignal : public Buffer<DataType> {
public:
    Buffer_WriteSignal_ReadSignal(int size, DataType value) : m_Size(size+1), m_Default(value) {
        m_Data = new DataType[m_Size];
        m_Read_Index = 1;
        m_Write_Index = 0;
        m_Init();
    }

    void Read(DataType *value) {
        *value = m_Data[m_Read_Index++];
        if (m_Read_Index == m_Size) m_Read_Index = 0;
    }

    void Write(DataType value) {
        m_Data[m_Write_Index++] = value;
        if (m_Write_Index == m_Size) m_Write_Index = 0;
    }

private:
    const int m_Size;
    DataType m_Default;
    DataType *m_Data;
    int m_Read_Index;
    int m_Write_Index;

    void m_Init(void) {
        for (int i = 0 ; i < m_Size; i++) {
            m_Data[i] = m_Default;
        }
    }
};

// ==================================================================
// SINGLE PROCESS WRITE SIGNAL THEN READ SIGNAL
// WITH MULTI PROCESS RESET
// ==================================================================

template <class DataType, class ResetReadLockType, class ResetWriteLockType>
class Buffer_WriteSignal_ReadSignal_Reset : public Buffer<DataType> {
public:
    Buffer_WriteSignal_ReadSignal_Reset(int size, DataType value, std::mutex *resetMutex) : m_Size(size+1), m_Default(value), m_ResetMutex(resetMutex) {
        m_Data = new DataType[m_Size];
        m_Read_Index = 1;
        m_Write_Index = 0;
        m_Init();
    }

    void Read(DataType *value) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            *value = m_Data[m_Read_Index++];
            if (m_Read_Index == m_Size) m_Read_Index = 0;
        }
    }

    void Write(DataType value) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            m_Data[m_Write_Index++] = value;
            if (m_Write_Index == m_Size) m_Write_Index = 0;
        }
    }

    void Reset(void) {
        sync::Synchronization<ResetWriteLockType> ResetSync(m_ResetMutex, ResetWriteLockType(), sync::scoped());
        if (ResetSync(ResetWriteLockType())) {
            m_Read_Index = 1;
            m_Write_Index = 0;
            m_Init();
        }
    }

private:
    const int m_Size;
    DataType m_Default;
    std::mutex *m_ResetMutex;

    DataType *m_Data;
    int m_Read_Index;
    int m_Write_Index;

    void m_Init(void) {
        for (int i = 0 ; i < m_Size; i++) {
            m_Data[i] = m_Default;
        }
    }
};


// ==================================================================
// SINGLE PROCESS READ SIGNAL THEN WRITE SIGNAL
// ==================================================================

template <class DataType>
class Buffer_ReadSignal_WriteSignal : public Buffer<DataType> {
public:
    Buffer_ReadSignal_WriteSignal(int size, DataType value) : m_Size(size), m_Default(value) {
        m_Data = new DataType[m_Size];
        m_Read_Index = 0;
        m_Write_Index = 0;
        m_Init();
    }

    void Read(DataType *value) {
        *value = m_Data[m_Read_Index++];
        if (m_Read_Index == m_Size) m_Read_Index = 0;
    }

    void Write(DataType value) {
        m_Data[m_Write_Index++] = value;
        if (m_Write_Index == m_Size) m_Write_Index = 0;
    }

private:
    const int m_Size;
    DataType m_Default;
    DataType *m_Data;
    int m_Read_Index;
    int m_Write_Index;

    void m_Init(void) {
        for (int i = 0 ; i < m_Size; i++) {
            m_Data[i] = m_Default;
        }
    }

};

// ==================================================================
// SINGLE PROCESS READ SIGNAL THEN WRITE SIGNAL
// WITH MULTI PROCESS RESET
// ==================================================================

template <class DataType, class ResetReadLockType, class ResetWriteLockType>
class Buffer_ReadSignal_WriteSignal_Reset : public Buffer<DataType> {
public:
    Buffer_ReadSignal_WriteSignal_Reset(int size, DataType value, std::mutex *resetMutex) : m_Size(size), m_Default(value), m_ResetMutex(resetMutex) {
        m_Data = new DataType[m_Size];
        m_Read_Index = 0;
        m_Write_Index = 0;
        m_Init();
    }

    void Read(DataType *value) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            *value = m_Data[m_Read_Index++];
            if (m_Read_Index == m_Size) m_Read_Index = 0;
        }
    }

    void Write(DataType value) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            m_Data[m_Write_Index++] = value;
            if (m_Write_Index == m_Size) m_Write_Index = 0;
        }
    }

    void Reset(void) {
        sync::Synchronization<ResetWriteLockType> ResetSync(m_ResetMutex, ResetWriteLockType(), sync::scoped());
        if (ResetSync(ResetWriteLockType())) {
            m_Read_Index = 0;
            m_Write_Index = 0;
            m_Init();
        }
    }

private:
    const int m_Size;
    DataType m_Default;
    std::mutex *m_ResetMutex;

    DataType *m_Data;
    int m_Read_Index;
    int m_Write_Index;

    void m_Init(void) {
        for (int i = 0 ; i < m_Size; i++) {
            m_Data[i] = m_Default;
        }
    }

};

// ==================================================================
// SINGLE PROCESS WRITE SIGNAL THEN READ BUNDLE
// ==================================================================

template <class DataType>
class Buffer_WriteSignal_ReadBundle : public Buffer<DataType> {
public:
    Buffer_WriteSignal_ReadBundle(int size, DataType value) : m_Size(size+1), m_Default(value) {
        m_Data = new DataType[m_Size];
        m_Write_Index = 0;
        m_Init();
    }

    void Read(DataType *value) {
        for (int i = 0 ; i < m_Size; i++) {
            value[i] = m_Data[m_Read_Index--];
            if (m_Read_Index < 0) m_Read_Index = m_Size - 1;
        }
    }

    void Write(DataType value) {
        m_Read_Index = m_Write_Index - 1;
        if (m_Read_Index < 0) m_Read_Index = m_Size - 1;
        m_Data[m_Write_Index++] = value;
        if (m_Write_Index == m_Size) m_Write_Index = 0;
    }

    void Reset(void) {
        m_Write_Index = 0;
        m_Init();
    }

private:
    const int m_Size;
    DataType m_Default;
    DataType *m_Data;
    int m_Read_Index;
    int m_Write_Index;

    void m_Init(void) {
        for (int i = 0 ; i < m_Size; i++) {
            m_Data[i] = m_Default;
        }
    }
};

// ==================================================================
// SINGLE PROCESS WRITE SIGNAL THEN READ BUNDLE
// WITH MULTI PROCESS RESET
// ==================================================================

template <class DataType, class ResetReadLockType, class ResetWriteLockType>
class Buffer_WriteSignal_ReadBundle_Reset : public Buffer<DataType> {
public:
    Buffer_WriteSignal_ReadBundle_Reset(int size, DataType value, std::mutex *resetMutex) : m_Size(size+1), m_Default(value), m_ResetMutex(resetMutex) {
        m_Data = new DataType[m_Size];
        m_Write_Index = 0;
        m_Init();
    }

    void Read(DataType *value) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            for (int i = 0 ; i < m_Size; i++) {
                value[i] = m_Data[m_Read_Index--];
                if (m_Read_Index < 0) m_Read_Index = m_Size - 1;
            }
        }
    }

    void Write(DataType value) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
            if (ResetSync(ResetReadLockType())) {
            m_Read_Index = m_Write_Index - 1;
            if (m_Read_Index < 0) m_Read_Index = m_Size - 1;
            m_Data[m_Write_Index++] = value;
            if (m_Write_Index == m_Size) m_Write_Index = 0;
        }
    }

    void Reset(void) {
        sync::Synchronization<ResetWriteLockType> ResetSync(m_ResetMutex, ResetWriteLockType(), sync::scoped());
        if (ResetSync(ResetWriteLockType())) {
            m_Write_Index = 0;
            m_Init();
        }
    }

private:
    const int m_Size;
    DataType m_Default;
    std::mutex *m_ResetMutex;

    DataType *m_Data;
    int m_Read_Index;
    int m_Write_Index;

    void m_Init(void) {
        for (int i = 0 ; i < m_Size; i++) {
            m_Data[i] = m_Default;
        }
    }
};

// ==================================================================
// SINGLE PROCESS READ BUNDLE THEN WRITE SIGNAL
// ==================================================================

template <class DataType>
class Buffer_ReadBundle_WriteSignal : public Buffer<DataType> {
public:
    Buffer_ReadBundle_WriteSignal(int size, DataType value) : m_Size(size), m_Default(value) {
        m_Data = new DataType[m_Size];
        m_Read_Index = m_Size - 1;
        m_Write_Index = 0;
        m_Init();
    }

    void Read(DataType value[]) {
        for (int i = 0 ; i < m_Size; i++) {
            value[i] = m_Data[m_Read_Index--];
            if (m_Read_Index < 0) m_Read_Index = m_Size - 1;
        }
    }

    void Write(DataType value) {
        m_Read_Index = m_Write_Index;
        m_Data[m_Write_Index++] = value;
        if (m_Write_Index == m_Size) m_Write_Index = 0;
    }

    void Reset(void) {
        m_Read_Index = m_Size - 1;
        m_Write_Index = 0;
        m_Init();
    }

private:
    const int m_Size;
    DataType m_Default;
    DataType *m_Data;
    int m_Read_Index;
    int m_Write_Index;

    void m_Init(void) {
        for (int i = 0 ; i < m_Size; i++) {
            m_Data[i] = m_Default;
        }
    }
};

// ==================================================================
// SINGLE PROCESS READ BUNDLE THEN WRITE SIGNAL
// WITH MULTI PROCESS RESET
// ==================================================================

template <class DataType, class ResetReadLockType, class ResetWriteLockType>
class Buffer_ReadBundle_WriteSignal_Reset : public Buffer<DataType> {
public:
    Buffer_ReadBundle_WriteSignal_Reset(int size, DataType value, std::mutex *resetMutex) : m_Size(size), m_Default(value), m_ResetMutex(resetMutex) {
        m_Data = new DataType[m_Size];
        m_Read_Index = m_Size - 1;
        m_Write_Index = 0;
        m_Init();
    }

    void Read(DataType *value) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            for (int i = 0 ; i < m_Size; i++) {
                value[i] = m_Data[m_Read_Index--];
                if (m_Read_Index < 0) m_Read_Index = m_Size - 1;
            }
        }
    }

    void Write(DataType value) {
        sync::Synchronization<ResetReadLockType> ResetSync(m_ResetMutex, ResetReadLockType(), sync::scoped());
        if (ResetSync(ResetReadLockType())) {
            m_Read_Index = m_Write_Index;
            m_Data[m_Write_Index++] = value;
            if (m_Write_Index == m_Size) m_Write_Index = 0;
        }
    }

    void Reset(void) {
        sync::Synchronization<ResetWriteLockType> ResetSync(m_ResetMutex, ResetWriteLockType(), sync::scoped());
        if (ResetSync(ResetWriteLockType())) {
            m_Read_Index = m_Size - 1;
            m_Write_Index = 0;
            m_Init();
        }
    }

private:
    const int m_Size;
    DataType m_Default;
    std::mutex *m_ResetMutex;

    DataType *m_Data;
    int m_Read_Index;
    int m_Write_Index;

    void m_Init(void) {
        for (int i = 0 ; i < m_Size; i++) {
            m_Data[i] = m_Default;
        }
    }
};

} // END NAMESPACE STRIDE

#endif // BUFFERCLASSES_HPP