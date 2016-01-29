#include "picin_core.h"

PicIn_Core::PicIn_Core()
{
    m_flagCancel = false;
    m_numFiles = 0;
}

// ****************************************************************************
// ****                       Public functions:                            ****
// ****************************************************************************

/*
 * name : scan_files
 * desc : Get number of files as specific extension from source path
 * ret  : Number of files from source path
 */
int PicIn_Core::scanSrcFiles(QStringList filters)
{
    m_fileInfoList_img.clear();
    m_fileInfoList_img = get_file_list(m_path_source, filters);
    m_numFiles = m_fileInfoList_img.size();

    return m_numFiles;
}

/*
 * name : getNumFilesSrc
 * desc : return number of files what just be scaned
 * ret  : Number of files from source path
 */
int PicIn_Core::getNumFilesSrc(void)
{
    return m_numFiles;
}

/*
 * name : set_path
 * desc : Set source or target path
 * in   :
 *   path, String of path
 *   pt, Set path to source or target
 * ret  :
 *    0, Success
 *   -1, Path doesn't exist
 *   -2, Invalid path type
 */
int PicIn_Core::set_path(QString path, PicIn_Core::PathType pt)
{
    if(pt != PT_Source && pt != PT_Target){
        return -2;
    }

    //
    // Check whether path has forward slash at last char
    //

    QSysInfo sysInfo;
    QString osVer = sysInfo.kernelType();

    QChar lastChar = path.at(path.size() - 1);
    QChar fSlash = QChar('/');

    if(!osVer.compare("linux", Qt::CaseInsensitive)){
        if(!operator ==(lastChar, fSlash)){
            path.append("/");
        }
    }

    //
    // Check path
    //

    QDir dir(path);
    QString *strPtr;

    if(!dir.exists()){
        return -1;
    }

    //
    // Set path
    //

    switch(pt){
    case PT_Source:
        strPtr = &m_path_source;
        break;

    case PT_Target:
        strPtr = &m_path_target;
        break;
    }

    strPtr->clear();
    strPtr->append(path);

    return 0;

}

/*
 * name : impport_doit
 * desc : Do import
 */
void PicIn_Core::import_doit()
{
    QFile file;
    QString fileName;
    QString filePath;

    m_flagCancel = false;

    //If we don;t do this proccessEvents(),
    //progress dialog would no response until this function done.
    QApplication::processEvents(QEventLoop::AllEvents);
    for(int i = 0; i < m_fileInfoList_img.size(); i++){
        fileName.clear();
        fileName.append(this->m_fileInfoList_img.at(i).fileName());

        filePath.clear();
        filePath.append(m_path_target);
        filePath.append(fileName);
        if(!file.exists(filePath)){
            file.copy(this->m_fileInfoList_img.at(i).absoluteFilePath(), filePath);
        }
        emit signal_update_progress(i);
        if(m_flagCancel){
            break;
        }

        QApplication::processEvents(QEventLoop::AllEvents);
    }

    m_flagCancel = false;
}

// ****************************************************************************
// ****                      Private functions:                            ****
// ****************************************************************************

/*
 * name : get_file_list
 * desc : Scan indicated path and return file info list of images
 * in   :
 *   path, String of source path
 *   filters, String list of file extension name
 * ret  : List of QFileInfo for all files under indicated path
 */
QFileInfoList PicIn_Core::get_file_list(
        QString path,
        QStringList filters
)
{
    QDir dir(path);
    QStringList nameFilters;

    //
    // Scan files
    //

    QFileInfoList fileInfoList;

    if(filters.size() > 0){
        nameFilters.append(filters);
    }
    dir.setFilter(QDir::NoSymLinks | QDir::Files);
    dir.setNameFilters(nameFilters);
    dir.setSorting(QDir::Name);

    fileInfoList = dir.entryInfoList();

    //
    // Scan sub directory
    //

    QFileInfoList dirInfoList;

    dir.setFilter(QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot);
    nameFilters.clear();
    dir.setNameFilters(nameFilters);
    dir.setSorting(QDir::Name);

    dirInfoList = dir.entryInfoList();

    for(int i = 0; i < dirInfoList.size(); i++){
        fileInfoList.append(
            get_file_list(dirInfoList.at(i).absoluteFilePath(),
            filters));
    }

    return fileInfoList;
}

// ****************************************************************************
// ****                         Public Slots:                              ****
// ****************************************************************************

/*
 * name : slot_set_flagCancel_true
 * desc : Set m_flagCancel to true
 */
void PicIn_Core::slot_set_flagCancel_true(void)
{
    PicIn_Core::m_flagCancel = true;
}
