#ifndef PICIN_CORE_H
#define PICIN_CORE_H

#include <QApplication>
#include <QFileInfoList>
#include <QDir>
#include <QDate>

class PicIn_Core : public QObject
{
    Q_OBJECT

public:

    enum PathType {
        PT_Source = 0,
        PT_Target = 1
    };

    PicIn_Core();

    int scanSrcFiles(QStringList filters);
    int getNumFilesSrc(void);
    int set_path(QString source, PathType pt);
    void import_doit();
    QDate getExifDate(QString path);
    int getBlEndInt32(char bl, u_int8_t *buf, int len);
    void setFlagDir(bool year, bool month, bool day);
    void setFlagCancel_true();

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
    bool m_flagDirYear;
    bool m_flagDirMon;
    bool m_flagDirDay;
    int m_numFiles;

signals:
    void signal_update_progress(int);

public slots:

};

#endif // PICIN_CORE_H
