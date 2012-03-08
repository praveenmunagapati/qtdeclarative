/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "private/qanimationgroupjob_p.h"

QT_BEGIN_NAMESPACE

QAnimationGroupJob::QAnimationGroupJob()
    : QAbstractAnimationJob(), m_firstChild(0), m_lastChild(0)
{
    m_isGroup = true;
}

QAnimationGroupJob::~QAnimationGroupJob()
{
    while (firstChild() != 0)
        delete firstChild();
}

void QAnimationGroupJob::topLevelAnimationLoopChanged()
{
    for (QAbstractAnimationJob *animation = firstChild(); animation; animation = animation->nextSibling())
        animation->topLevelAnimationLoopChanged();
}

void QAnimationGroupJob::appendAnimation(QAbstractAnimationJob *animation)
{
    if (QAnimationGroupJob *oldGroup = animation->m_group)
        oldGroup->removeAnimation(animation);

    Q_ASSERT(!animation->previousSibling() && !animation->nextSibling());

    if (m_lastChild)
        m_lastChild->m_nextSibling = animation;
    else
        m_firstChild = animation;
    animation->m_previousSibling = m_lastChild;
    m_lastChild = animation;

    animation->m_group = this;
    animationInserted(animation);
}

void QAnimationGroupJob::prependAnimation(QAbstractAnimationJob *animation)
{
    if (QAnimationGroupJob *oldGroup = animation->m_group)
        oldGroup->removeAnimation(animation);

    Q_ASSERT(!previousSibling() && !nextSibling());

    if (m_firstChild)
        m_firstChild->m_previousSibling = animation;
    else
        m_lastChild = animation;
    animation->m_nextSibling = m_firstChild;
    m_firstChild = animation;

    animation->m_group = this;
    animationInserted(animation);
}

void QAnimationGroupJob::removeAnimation(QAbstractAnimationJob *animation)
{
    Q_ASSERT(animation);
    Q_ASSERT(animation->m_group == this);
    QAbstractAnimationJob *prev = animation->previousSibling();
    QAbstractAnimationJob *next = animation->nextSibling();

    if (prev)
        prev->m_nextSibling = next;
    else
        m_firstChild = next;

    if (next)
        next->m_previousSibling = prev;
    else
        m_lastChild = prev;

    animation->m_previousSibling = 0;
    animation->m_nextSibling = 0;

    animation->m_group = 0;
    animationRemoved(animation, prev, next);
}

void QAnimationGroupJob::clear()
{
    //### should this remove and delete, or just remove?
    while (firstChild() != 0)
        delete firstChild(); //removeAnimation(firstChild());
}

void QAnimationGroupJob::resetUncontrolledAnimationsFinishTime()
{
    for (QAbstractAnimationJob *animation = firstChild(); animation; animation = animation->nextSibling()) {
        if (animation->duration() == -1 || animation->loopCount() < 0) {
            resetUncontrolledAnimationFinishTime(animation);
        }
    }
}

void QAnimationGroupJob::resetUncontrolledAnimationFinishTime(QAbstractAnimationJob *anim)
{
    setUncontrolledAnimationFinishTime(anim, -1);
}

void QAnimationGroupJob::setUncontrolledAnimationFinishTime(QAbstractAnimationJob *anim, int time)
{
    anim->m_uncontrolledFinishTime = time;
}

void QAnimationGroupJob::uncontrolledAnimationFinished(QAbstractAnimationJob *animation)
{
    Q_UNUSED(animation);
}

void QAnimationGroupJob::animationRemoved(QAbstractAnimationJob* anim, QAbstractAnimationJob* , QAbstractAnimationJob* )
{
    resetUncontrolledAnimationFinishTime(anim);
    if (!firstChild()) {
        m_currentTime = 0;
        stop();
    }
}

QT_END_NAMESPACE