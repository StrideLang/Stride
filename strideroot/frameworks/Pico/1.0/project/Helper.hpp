#ifndef HELPER_HPP
#define HELPER_HPP

namespace stride {

template <class DataType>
class SignalHelper {
public:
    SignalHelper(DataType defaultValue = DataType()) {
        m_DefaultValue = defaultValue;
        init_Internal();
    }

    void process_OutputDomain(DataType *Output, DataType External) {
        m_Internal += External;
        *Output = m_Internal;
    }

    void process_PortDomain(DataType Input, DataType *Output) {
        *Output = Input;
    }

    void init_External(DataType *Output) {
        *Output = m_DefaultValue;
    }

    void init_Internal(void) {
        m_Internal = m_DefaultValue;
    }

private:
    DataType m_Internal;
    DataType m_DefaultValue;
};


template <class DataType>
class BundleHelper {
public:
    BundleHelper(int size) : m_Size(size) {
        m_Internal = new DataType[m_Size];
        init_Internal();
    }

    void process_OutputDomain(DataType *Output, DataType External[]) {
        DataType Sum = (DataType) 0.0;
        for (int i = 0; i < m_Size; i++) {
            m_Internal[i] += External[i];
            Sum += m_Internal[i];
        }
        *Output = Sum;
    }

    void process_PortDomain(DataType Input[], DataType *Output) {
        for (int i = 0; i < m_Size; i++) {
            Output[i] = Input[i];
        }
    }

    void init_External(DataType *Output) {
        for (int i = 0; i < m_Size; i++) {
            Output[i] = (DataType) 1.0;
        }
    }

    void init_Internal(void) {
        for (int i = 0; i < m_Size; i++) {
            m_Internal[i] = 0.0;
        }
    }

private:
    const int m_Size;
    DataType *m_Internal;
};

}

#endif // HELPER_HPP
