/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "signalhandlerproperty.h"
#include "internalproperty.h"
#include "invalidmodelnodeexception.h"
#include "invalidpropertyexception.h"
#include "invalidargumentexception.h"
#include "internalnode_p.h"
#include "model.h"
#include "model_p.h"
namespace QmlDesigner {

SignalHandlerProperty::SignalHandlerProperty() = default;

SignalHandlerProperty::SignalHandlerProperty(const SignalHandlerProperty &property, AbstractView *view)
    : AbstractProperty(property.name(), property.internalNode(), property.model(), view)
{
}

SignalHandlerProperty::SignalHandlerProperty(const PropertyName &propertyName, const Internal::InternalNodePointer &internalNode, Model* model, AbstractView *view)
    : AbstractProperty(propertyName, internalNode, model, view)
{
}

void SignalHandlerProperty::setSource(const QString &source)
{
    Internal::WriteLocker locker(model());
    if (!isValid())
        throw InvalidModelNodeException(__LINE__, __FUNCTION__, __FILE__);

    if (name() == "id") { // the ID for a node is independent of the state, so it has to be set with ModelNode::setId
        throw InvalidPropertyException(__LINE__, __FUNCTION__, __FILE__, name());
    }

    if (source.isEmpty())
        throw InvalidArgumentException(__LINE__, __FUNCTION__, __FILE__, name());

    if (internalNode()->hasProperty(name())) { //check if oldValue != value
        Internal::InternalProperty::Pointer internalProperty = internalNode()->property(name());
        if (internalProperty->isSignalHandlerProperty()
            && internalProperty->toSignalHandlerProperty()->source() == source)

            return;
    }

    if (internalNode()->hasProperty(name()) && !internalNode()->property(name())->isSignalHandlerProperty())
        privateModel()->removeProperty(internalNode()->property(name()));

    privateModel()->setSignalHandlerProperty(internalNode(), name(), source);
}

QString SignalHandlerProperty::source() const
{
    if (internalNode()->hasProperty(name())
        && internalNode()->property(name())->isSignalHandlerProperty())
        return internalNode()->signalHandlerProperty(name())->source();

    return QString();
}

PropertyName SignalHandlerProperty::prefixAdded(const PropertyName &propertyName)
{
    QString nameAsString = QString::fromUtf8(propertyName);
    if (nameAsString.startsWith("on"))
        return propertyName;

    QChar firstChar = nameAsString.at(0).toUpper();
    nameAsString[0] = firstChar;
    nameAsString.prepend("on");

    return nameAsString.toLatin1();
}

PropertyName SignalHandlerProperty::prefixRemoved(const PropertyName &propertyName)
{
    QString nameAsString = QString::fromUtf8(propertyName);
    if (!nameAsString.startsWith("on"))
        return propertyName;

    nameAsString.remove(0, 2);
    QChar firstChar = nameAsString.at(0).toLower();
    nameAsString[0] = firstChar;

    return nameAsString.toLatin1();
}

SignalDeclarationProperty::SignalDeclarationProperty() = default;

SignalDeclarationProperty::SignalDeclarationProperty(const SignalDeclarationProperty &property,
                                                     AbstractView *view)
    : AbstractProperty(property.name(), property.internalNode(), property.model(), view)
{}

SignalDeclarationProperty::SignalDeclarationProperty(
    const PropertyName &propertyName,
    const Internal::InternalNodePointer &internalNode,
    Model *model,
    AbstractView *view)
    : AbstractProperty(propertyName, internalNode, model, view)
{
}

void SignalDeclarationProperty::setSignature(const QString &signature)
{
    Internal::WriteLocker locker(model());
    if (!isValid())
        throw InvalidModelNodeException(__LINE__, __FUNCTION__, __FILE__);

    if (name() == "id") { // the ID for a node is independent of the state, so it has to be set with ModelNode::setId
        throw InvalidPropertyException(__LINE__, __FUNCTION__, __FILE__, name());
    }

    if (signature.isEmpty())
        throw InvalidArgumentException(__LINE__, __FUNCTION__, __FILE__, name());

    if (internalNode()->hasProperty(name())) { //check if oldValue != value
        Internal::InternalProperty::Pointer internalProperty = internalNode()->property(name());
        if (internalProperty->isSignalDeclarationProperty()
            && internalProperty->toSignalDeclarationProperty()->signature() == signature)

            return;
    }

    if (internalNode()->hasProperty(name()) && !internalNode()->property(name())->isSignalDeclarationProperty())
        privateModel()->removeProperty(internalNode()->property(name()));

    privateModel()->setSignalDeclarationProperty(internalNode(), name(), signature);
}

QString SignalDeclarationProperty::signature() const
{

    if (internalNode()->hasProperty(name())
        && internalNode()->property(name())->isSignalDeclarationProperty())
        return internalNode()->signalDeclarationProperty(name())->signature();

    return QString();
}

} // namespace QmlDesigner
