#ifndef PICIN_CORE_H
#define PICIN_CORE_H

#include <QApplication>
#include <QFileInfoList>
#include <QDir>

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

private:
    QFileInfoList get_file_list(QString path, QStringList filters);

    QFileInfoList m_fileInfoList_img;
    QString m_path_source;
    QString m_path_target;
    bool m_flagCancel;
    int m_numFiles;

signals:
    void signal_update_progress(int);

public slots:
    void slot_set_flagCancel_true();

};

#endif // PICIN_CORE_H
