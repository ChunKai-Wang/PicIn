#ifndef PICIN_CORE_H
#define PICIN_CORE_H

#include <QApplication>
#include <QFileInfoList>
#include <QDir>
#include <QDate>
#include <QProcess>
#include <QDebug>

//
// OS specific including
//

#ifdef Q_OS_LINUX
#include "utime.h"
#endif

#ifdef Q_OS_WIN
#include "stdint.h"
#endif

//
// OS specific define
//

#ifdef Q_OS_WIN
#define FILE_PATH_SEPARATOR "\\"
#define MODEL_FS_INDEX_TYPE_FOLDER "File Folder"
#endif

#ifdef Q_OS_LINUX
#define FILE_PATH_SEPARATOR "/"
#define MODEL_FS_INDEX_TYPE_FOLDER "Folder"
#endif

#ifdef Q_OS_WIN
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

    enum Option{
        optionCancel = 0x01,
        optionExifDate = 0x02,
        optionDirYear = 0x04,
        optionDirMon = 0x08,
        optionDirDay = 0x10,
        optionSubDir = 0x20,
        optionOverwrite = 0x40
    };
    Q_DECLARE_FLAGS(Options, Option)

    PicIn_Core();

    int scanSrcFiles(QStringList filters);
    int getNumFilesSrc(void);
    int set_path(QStringList pathList, PathType pt);
    void import_doit();
    QDate getExifDate(QString path);
    int getBlEndInt32(char bl, u_int8_t *buf, int len);

    void setFlagExifDate(bool flag);
    void setFlagDir(bool year, bool month, bool day);
    void setFlagCancel_true();
    void setFlagSubDir(bool flag);
    void setFlagOverwrite(bool flag);

    int setLastModifyDateTime(QString path, QDateTime laDateTime, QDateTime lmDateTime);

    void setOption(Options options);
    PicIn_Core::Options getOptions(void);
    void onOption(Options on);
    void offOption(Options off);
    bool checkOption(Options check);

private:

    typedef struct{
        u_int8_t tagNum[2];
        u_int8_t dataFmt[2];
        u_int8_t numOfComponent[4];
        u_int8_t data[4];
    }IfdEntry;

    QFileInfoList get_file_list(QStringList pathList, QStringList filters);
    QFileInfoList get_file_list(QString path, QStringList filters);
    void get_copyTgt_List(void);

    QFileInfoList m_fileInfoList_src;
    QStringList m_copyTgtList;
    QStringList m_pathList_source;
    QStringList m_pathList_target;

    PicIn_Core::Options m_options;

    int m_numFiles;
    QProcess m_process;

signals:
    void signal_update_progress(int);

public slots:

};

Q_DECLARE_OPERATORS_FOR_FLAGS(PicIn_Core::Options)

#endif // PICIN_CORE_H
