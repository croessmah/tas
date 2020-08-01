#pragma once
#ifndef INCLUDE_OPERATOR_EYE_CONTROLLERS_GROUP_H
#define INCLUDE_OPERATOR_EYE_CONTROLLERS_GROUP_H
#include <qobject.h>

namespace eye
{
    class controller;

    class controllers_group: public QObject
    {
        Q_OBJECT
    public:
        controllers_group(unsigned _wait_every, QObject * _parent);
        ~controllers_group();        
        controller & get_srgm() const;
        controller & get_ft5p() const;
    signals:
        void changed();
    public slots:
        void update_all();
    private slots:
        void update_completed(bool _has_change);
    private:
        controller * m_srgm;
        controller * m_ft5p;
        unsigned m_timeout;
        bool m_changed;
    };


}//namespace eye


#endif //INCLUDE_OPERATOR_EYE_CONTROLLERS_GROUP_H
