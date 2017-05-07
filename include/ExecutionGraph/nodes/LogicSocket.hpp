// ========================================================================================
//  ExecutionGraph
//  Copyright (C) 2014 by Gabriel Nützi <gnuetzi (at) gmail (døt) com>
//
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================================

#ifndef ExecutionGraph_nodes_LogicSocket_hpp
#define ExecutionGraph_nodes_LogicSocket_hpp

#include <unordered_set>
#include <meta/meta.hpp>

#include "ExecutionGraph/common/Asserts.hpp"
#include "ExecutionGraph/common/DemangleTypes.hpp"
#include "ExecutionGraph/common/EnumClassHelper.hpp"
#include "ExecutionGraph/common/TypeDefs.hpp"
#include "ExecutionGraph/nodes/LogicCommon.hpp"

namespace executionGraph
{
//! The socket base class for all input/output sockets of a logic node.
template<typename TConfig>
class LogicSocketBase
{
public:
    EXEC_GRAPH_TYPEDEF_CONFIG(TConfig);

    LogicSocketBase(IndexType type,
                    SocketIndex index,
                    NodeBaseType& parent,
                    const std::string& name = "")
        : m_type(type), m_index(index), m_parent(parent), m_name( (name.empty())? name : "[" + std::to_string(index) + "]")
    {
    }

    IndexType getType() const { return m_type; }
    SocketIndex getIndex() const { return m_index; }
    std::string getName() const { return m_name; }

    const NodeBaseType& getParent() const { return m_parent; }
    NodeBaseType& getParent() { return m_parent; }

protected:
    const IndexType m_type;     //!< The index in to the meta::list SocketTypes, which type this is!
    const SocketIndex m_index;  //!< The index of the slot at which this socket is installed in a LogicNode.
    NodeBaseType& m_parent;     //!< The parent logic node of of this socket.
    const std::string m_name;   //!< The name of the socket.

    std::string getNameOfType() const;  //!< Returns the name of the current type.
};

template<typename TConfig>
std::string LogicSocketBase<TConfig>::getNameOfType() const
{
    std::string s = "'type-not-found'";
    IndexType i   = 0;
    auto f        = [&](auto type) {
        if (i == m_type)
        {
            s = demangle(type);
        }
        i++;
    };

    meta::for_each(SocketTypes{}, f);
    return s;
}

//! The input socket base class for all input/output sockets of a logic node.
template<typename TConfig>
class LogicSocketInputBase : public LogicSocketBase<TConfig>
{
public:
    EXEC_GRAPH_TYPEDEF_CONFIG(TConfig);

    friend class LogicSocketOutputBase<Config>;

    template<typename... Args>
    LogicSocketInputBase(Args&&... args)
        : LogicSocketBase<TConfig>(std::forward<Args>(args)...)
    {
    }

    //! Cast to a logic socket of type \p SocketInputType<T>*.
    //! The cast fails at runtime if the data type \p T does not match!
    template<typename T>
    inline auto* castToType() const
    {
        EXEC_GRAPH_THROW_BADSOCKETCAST_IF((this->m_type != meta::find_index<SocketTypes, T>::value),
                                          "Casting socket index '" << this->m_index << "' with type '" << this->getNameOfType() << "' into '"
                                                                   << demangle<T>()
                                                                   << "' of logic node id: '"
                                                                   << this->m_parent.getId()
                                                                   << "' which is wrong!");

        return static_cast<SocketInputType<T> const*>(this);
    }

    //! Non-const overload.
    template<typename T>
    inline auto* castToType()
    {
        return const_cast<SocketInputType<T>*>(static_cast<LogicSocketInputBase const*>(this)->castToType<T>());
    }

    void setGetLink(LogicSocketOutputBase<Config>& outputSocket);

    //! Check if the socket has a Get-Link to an output socket.
    inline bool hasGetLink() const { return m_getFrom != nullptr; }

    inline LogicSocketOutputBase<Config>* followGetLink() { return m_getFrom; }

    const auto& getWritingSockets(){ return m_writingParents; }
    IndexType getConnectionCount(){ return (hasGetLink()? 1 : 0) + m_writingParents.size(); }

protected:
    LogicSocketOutputBase<Config>* m_getFrom = nullptr;  //!< The single Get-Link attached to this Socket.
    std::unordered_set<LogicSocketOutputBase<Config>*> m_writingParents; //!< All parent output sockets which write to this input.
};

//! The input socket base class for all input/output sockets of a logic node.
template<typename TConfig>
class LogicSocketOutputBase : public LogicSocketBase<TConfig>
{
public:
    EXEC_GRAPH_TYPEDEF_CONFIG(TConfig);
    friend class LogicSocketInputBase<Config>;

    template<typename... Args>
    LogicSocketOutputBase(Args&&... args)
        : LogicSocketBase<TConfig>(std::forward<Args>(args)...)
    {
    }

    //! Cast to a logic socket of type \p SocketOutputType<T>*.
    //! The cast fails at runtime (if NDEBUG defined) if the data type \p T does not match!
    template<typename T>
    inline auto* castToType() const
    {
        EXEC_GRAPH_THROW_BADSOCKETCAST_IF((this->m_type != meta::find_index<SocketTypes, T>::value),
                                          "Casting socket index '" << this->m_index << "' with type '" << this->getNameOfType() << "' into '"
                                                                   << demangle<T>()
                                                                   << "' of logic node id: '"
                                                                   << this->m_parent.getId()
                                                                   << "' which is wrong!");

        return static_cast<SocketOutputType<T> const*>(this);
    }

    //! Non-const overload.
    template<typename T>
    inline auto* castToType()
    {
        return const_cast<SocketOutputType<T>*>(static_cast<LogicSocketOutputBase const*>(this)->castToType<T>());
    }

    void addWriteLink(LogicSocketInputBase<Config>& inputSocket);

    const auto& getGetterSockets(){ return m_getterChilds; }
    IndexType getConnectionCount(){ return m_writeTo.size() + m_getterChilds.size(); }

protected:
    std::vector<LogicSocketInputBase<Config>*> m_writeTo;  //!< All Write-Links attached to this Socket.
    std::unordered_set<LogicSocketInputBase<Config>*> m_getterChilds; //!< All child sockets which have a Get-Link to this socket.
};

template<typename TData>
class LogicSocketData
{
public:
    using DataType = TData;

    template<typename T>
    LogicSocketData(T&& defaultValue)
        : m_data(std::forward<T>(defaultValue))
    {
    }

    //! Get the data value of the socket.
    inline const DataType& getValue() const { return m_data; }
    //! Non-const overload.
    inline DataType& getValue() { return m_data; }

    //! Set the data value of the socket.
    template<typename T>
    void setValue(T&& value)
    {
        m_data = std::forward<T>(value);
    }

protected:
    DataType m_data;  //!< Default value! or the output value if output socket
};

template<typename TData, typename TConfig>
class LogicSocketInput : public LogicSocketInputBase<TConfig>,
                         public LogicSocketData<TData>
{
public:
    EXEC_GRAPH_TYPEDEF_CONFIG(TConfig);
    using DataType = TData;

    /** This assert fails if the type T of the LogicSocket is
        not properly added to the type list SocketTypes in LogicSocketBase*/
    static_assert(!std::is_same<meta::find<SocketTypes, DataType>, meta::list<>>::value,
                  "TData is not in SocketTypes!");

    template<typename T, typename... Args>
    LogicSocketInput(T&& defaultValue, Args&&... args)
        : LogicSocketInputBase<TConfig>(meta::find_index<SocketTypes, DataType>::value, std::forward<Args>(args)...)
        , LogicSocketData<TData>(std::forward<T>(defaultValue))
    {
    }

    //! Get the data value of the socket. (follow Get-Link)
    inline const DataType& getValue() const
    {
        if (this->m_getFrom != nullptr)
        {
            return static_cast<LogicSocketOutput<DataType, Config>*>(this->m_getFrom)->getValue();
        }
        else
        {
            return this->m_data;
        }
    }
    //! Non-const overload.
    inline DataType& getValue()
    {
        return const_cast<DataType&>(static_cast<const LogicSocketInput*>(this)->getValue());
    }
};

template<typename TData, typename TConfig>
class LogicSocketOutput : public LogicSocketOutputBase<TConfig>,
                          public LogicSocketData<TData>
{
public:
    EXEC_GRAPH_TYPEDEF_CONFIG(TConfig);

    using DataType = TData;
    /** This assert fails if the type T of the LogicSocket is
        not properly added to the type list SocketTypes in LogicSocketBase*/
    static_assert(!std::is_same<meta::find<SocketTypes, DataType>, meta::list<>>::value,
                  "TData is not in SocketTypes!");

    template<typename T, typename... Args>
    LogicSocketOutput(T&& defaultValue, Args&&... args)
        : LogicSocketOutputBase<TConfig>(meta::find_index<SocketTypes, DataType>::value, std::forward<Args>(args)...)
        , LogicSocketData<TData>(std::forward<T>(defaultValue))
    {
    }
    //! Set the data value of the socket.
    template<typename T>
    void setValue(T&& value)
    {
        // Set the value
        LogicSocketData<TData>::setValue(std::forward<T>(value));
        // Forward the value to all Write-Links
        executeWriteLinks();
    }

    void executeWriteLinks();
};

}  // end ExecutionGraph

// =====================================================================
// Implementation
// =====================================================================
namespace executionGraph
{
template<typename TConfig>
void LogicSocketOutputBase<TConfig>::addWriteLink(LogicSocketInputBase<TConfig>& inputSocket)
{

    EXEC_GRAPH_THROWEXCEPTION_TYPE_IF(inputSocket.getParent().getId() == this->getParent().getId(),
                                      "No Write-Link connection to our input slot! (node id: " << this->getParent().getId() << ")",
                                      NodeConnectionException);

    EXEC_GRAPH_THROWEXCEPTION_TYPE_IF(this->getType() != inputSocket.getType(),
                                      "Output socket: " << this->getName() << " of logic node id: " << this->getParent().getId()
                                                        << " has not the same type as input socket "
                                                        << inputSocket.getName()
                                                        << " of logic node id: "
                                                        << inputSocket.getParent().getId(),
                                      NodeConnectionException);

    EXEC_GRAPH_THROWEXCEPTION_TYPE_IF(m_getterChilds.find(&inputSocket) != m_getterChilds.end(),
                                      "Cannot add Write-Link from output socket: "
                                      << this->getName() << " of logic node id: " << this->getParent().getId()
                                      << " to "
                                      << inputSocket.getName() << " of logic node id: " << inputSocket.getParent().getId()
                                      << "because input already has a Get-Link to this output.",
                                      NodeConnectionException);

    if (std::find(m_writeTo.begin(), m_writeTo.end(), &inputSocket) == m_writeTo.end())
    {
        m_writeTo.push_back(&inputSocket);
        inputSocket.m_writingParents.emplace(this);
    }
    else
    {
        EXEC_GRAPH_THROWEXCEPTION_TYPE("Input socket: " << inputSocket.getName() << " of logic node id: "
                                                        << inputSocket.getParent().getId()
                                                        << " already added as Write-Link to logic node id: "
                                                        << this->getParent().getId(),
                                       NodeConnectionException);
    }
}

template<typename TConfig>
void LogicSocketInputBase<TConfig>::setGetLink(LogicSocketOutputBase<TConfig>& outputSocket)
{

    EXEC_GRAPH_THROWEXCEPTION_TYPE_IF(outputSocket.getParent().getId() == this->getParent().getId(),
                                      "No Get-Link connection to our output slot! (node id: " << this->getParent().getId() << ")",
                                      NodeConnectionException);

    EXEC_GRAPH_THROWEXCEPTION_TYPE_IF(this->getType() != outputSocket.getType(),
                                      "Output socket: " << outputSocket.getName() << " of logic node id: " << outputSocket.getParent().getId()
                                      << " has not the same type as input socket "
                                      << this->getName()
                                      << " of logic node id: "
                                      << this->getParent().getId(),
                                      NodeConnectionException);

    EXEC_GRAPH_THROWEXCEPTION_TYPE_IF(m_writingParents.find(&outputSocket) != m_writingParents.end(),
                                  "Cannot add Get-Link from input socket: "
                                  << this->getName()<< " of logic node id: " << this->getParent().getId()
                                  << " to "
                                  << outputSocket.getName() << " of logic node id: " << outputSocket.getParent().getId()
                                  << "because output already has a Write-Link to this input.",
                                  NodeConnectionException);

    if (!hasGetLink())
    {
        //std::cout << "Add Get-Link: " << outputSocket.getParent().getId() << outputSocket.getName() << " --> " << this->getParent().getId() << this->getName() << std::endl;
        m_getFrom = &outputSocket;
        outputSocket.m_getterChilds.emplace(this);
    }
    else
    {
        EXEC_GRAPH_THROWEXCEPTION_TYPE("Get-Link of logic node id: " << this->getParent().getId()
                                       << " already set!",
                                       NodeConnectionException);
    }
}

template<typename TData, typename TConfig>
void LogicSocketOutput<TData, TConfig>::executeWriteLinks()
{
    // Write out value to all connected (Write-Link) input sockets.
    for (auto& inputSocket : this->m_writeTo)
    {
        // We know that this static cast is safe, since it has been
        // checked when addWriteLink() is called.
        static_cast<LogicSocketInput<TData, Config>*>(this->s)->setValue(this->m_data);  // Write data to input sockets.
    }
}

}  // end executionGraph
#endif
