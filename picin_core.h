#ifndef PICIN_CORE_H
#define PICIN_CORE_H

#include <QApplication>
#include <QFileInfoList>
#include <QDir>
#include <QDate>
#include <QProcess>

#ifdef Q_OS_WIN
#include "stdint.h"
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
#endif

class PicIn_Core : public QObject
{
    Q_OBJECT

public:

    enum PathType {
        PT_Source = 0,
        PT_Target = 1
    };

    enum OsType{
        OS_LINUX = 0,
        OS_WIN = 1
    };

    PicIn_Core();

    int scanSrcFiles(QStringList filters);
    int getNumFilesSrc(void);
    int set_path(QString source, PathType pt);
    void import_doit();
    QDate getExifDate(QString path);
    int getBlEndInt32(char bl, u_int8_t *buf, int len);

    void setFlagExifDate(bool flag);
    void setFlagDir(bool year, bool month, bool day);
    void setFlagCancel_true();
    void setFlagSubDir(bool flag);
    void setFlagOverwrite(bool flag);

    void setLastModifyDateTime(QString path, QDate date, QTime time);

private:

    typedef struct{
        u_int8_t tagNum[2];
        u_int8_t dataFmt[2];
        u_int8_t numOfComponent[4];
        u_int8_t data[4];
    }IfdEntry;

    QFileInfoList get_file_list(QString path, QStringList filters);

    QFileInfoList m_fileInfoList_img;
    QString m_path_source;
    QString m_path_target;

    bool m_flagCancel;
    bool m_flagExifDate;
    bool m_flagDirYear;
    bool m_flagDirMon;
    bool m_flagDirDay;
    bool m_flagSubDir;
    bool m_flagOverwrite;

    int m_numFiles;
    int m_os;
    QProcess m_process;

signals:
    void signal_update_progress(int);

public slots:

};

#endif // PICIN_CORE_H
