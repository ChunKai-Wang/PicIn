#include "picin_core.h"

PicIn_Core::PicIn_Core()
{
    m_flagCancel = false;
    m_flagDirYear = false;
    m_flagDirMon = false;
    m_flagDirDay = false;
    m_flagSubDir = false;
    m_flagOverwrite = false;
    m_numFiles = 0;

#ifdef Q_OS_LINUX
    m_os = OS_LINUX;
#endif
#ifdef Q_OS_WIN
    m_os = OS_WIN;
#endif
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

    QChar lastChar = path.at(path.size() - 1);;
    QString slash;

    switch(m_os){
    case OS_LINUX:
    default:
        slash.sprintf("%s", "/");
        break;

    case OS_WIN:
        slash.sprintf("%s", "\\");
        break;
    }

    if(!operator ==(lastChar, slash.at(0))){
        path.append(slash);
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
 * name : setLastModifyDateTime
 * desc : Set indicated file's last modified time
 */
void PicIn_Core::setLastModifyDateTime(QString path, QDate date, QTime time)
{
    if(m_os == OS_LINUX){
        QString cmd;
        cmd.sprintf("touch -t %04d%02d%02d%02d%02d ",
                    date.year(),
                    date.month(),
                    date.day(),
                    time.hour(),
                    time.minute());
        cmd.append(path);

        m_process.start(cmd);
        while(m_process.state() != QProcess::NotRunning){
            QApplication::processEvents(QEventLoop::AllEvents);
        }
    }

}

/*
 * name : impport_doit
 * desc : Do import
 */
void PicIn_Core::import_doit()
{
    QFile file;
    QString tgtName;
    QString tgtPath;
    QString yearPath;
    QString monthPath;
    QString dayPath;
    QString srcPath;
    QDate date;
    QDir dir;

    m_flagCancel = false;

    //If we don't do this proccessEvents(),
    //progress dialog would no response until this function done.
    QApplication::processEvents(QEventLoop::AllEvents);
    for(int i = 0; i < m_fileInfoList_img.size(); i++){
        srcPath = this->m_fileInfoList_img.at(i).absoluteFilePath();

        tgtName.clear();
        tgtName.append(this->m_fileInfoList_img.at(i).fileName());

        tgtPath.clear();
        tgtPath.append(m_path_target);

        //
        // Check whether need to separate pics to folders as date
        //

        if(m_flagExifDate){
            date = getExifDate(srcPath);
        }
        if(!date.isValid() || !m_flagExifDate){
            date = this->m_fileInfoList_img.at(i).lastModified().date();
        }

        yearPath.clear();
        monthPath.clear();
        dayPath.clear();
        if(m_flagDirYear){
            yearPath.sprintf("/%04d/", date.year());
            tgtPath.append(yearPath);
            if(!dir.exists(tgtPath)){
                dir.mkdir(tgtPath);
            }
        }
        if(m_flagDirMon){
            monthPath.sprintf("/%02d/", date.month());
            tgtPath.append(monthPath);
            if(!dir.exists(tgtPath)){
                dir.mkdir(tgtPath);
            }
        }
        if(m_flagDirDay){
            dayPath.sprintf("/%02d/", date.day());
            tgtPath.append(dayPath);
            if(!dir.exists(tgtPath)){
                dir.mkdir(tgtPath);
            }
        }

        //
        // Copy file
        //

        tgtPath.append(tgtName);
        if(!file.exists(tgtPath) || m_flagOverwrite){
            file.copy(srcPath, tgtPath);
            setLastModifyDateTime(
                tgtPath,
                m_fileInfoList_img.at(i).lastModified().date(),
                m_fileInfoList_img.at(i).lastModified().time());
        }

        //
        // Update progress bar and check cancel
        //

        emit signal_update_progress(i);
        if(m_flagCancel){
            break;
        }

        QApplication::processEvents(QEventLoop::AllEvents);
    }

    m_flagCancel = false;
}

/*
 * name : getExifDate
 * desc : get date time info from indicated file's exif
 */
QDate PicIn_Core::getExifDate(QString path)
{
    QFile file(path);
    int y = 0;
    int m = 0;
    int d = 0;

    char blEnd = 'l';

    u_int8_t buf[4096];

    u_int16_t offset_ffe1 = 0;
    u_int16_t offset_lenOfFfe1 = 0;

    u_int16_t offset_ffe0 = 0;
    u_int16_t offset_lenOfFfe0 = 0;
    u_int16_t len_ffe0 = 0;

    u_int16_t offset_exifHeader = 0;
    u_int16_t offset_tiffHeader = 0;

    u_int32_t offset_exifSubIfd = 0;

    u_int32_t offset_numOfIfd0Entry = 0;
    u_int16_t numOfIfdEntry = 0;

    IfdEntry *ifdEntry = NULL;

    u_int32_t offset_orgDate = 0;

    int i = 0;
    int multiplier = 0;

    //
    // Start
    //

    file.open(QIODevice::ReadOnly);

    file.read((char *)buf, 4096);

    //
    // Check 0xFFD8(Start Of Image)
    //

    if(buf[0] != 0xff && buf[1] != 0xd8){
        goto errout;
    }

    //
    // Check 0xFFE1, APP1 marker for exif
    //

    if(buf[2] == 0xff){
        if(buf[3] == 0xe1){

            //
            // Found 0xFFE1, the APP1, exif marker.
            //

            offset_ffe1 = 2;
        }
        else if(buf[3] == 0xe0){

            //
            // Found 0xFFE0, APP0, JFIF marker,
            // the next marker should be 0xFFE1 APP1, the exif marker what we want.
            //

            offset_ffe0 = 2;
            offset_lenOfFfe0 = offset_ffe0 + 2;
            len_ffe0 = buf[offset_lenOfFfe0] * 0x10 + buf[offset_lenOfFfe0 + 1];

            if(buf[offset_lenOfFfe0 + len_ffe0] == 0xff && buf[offset_lenOfFfe0 + len_ffe0 + 1] == 0xe1){
                offset_ffe1 = offset_lenOfFfe0 + len_ffe0;
            }
            else{
                goto errout;
            }
        }
        else{
            goto errout;
        }
    }
    else{
        goto errout;
    }

    offset_lenOfFfe1 = offset_ffe1 + 2;
    offset_exifHeader = offset_lenOfFfe1 + 2;
    offset_tiffHeader = offset_exifHeader + 6;

    //
    // Check tiff header
    //

    if(buf[offset_tiffHeader] == 0x49 && buf[offset_tiffHeader + 1] == 0x49){
        // little endian, 0x0A0B0C0D will be [0]:0x0D,[1]:0x0C,[2]:0x0B,[3]:0x0A
        blEnd = 'l';
    }
    else if(buf[offset_tiffHeader] == 0x4d && buf[offset_tiffHeader + 1] == 0x4d){
        // big endian, 0x0A0B0C0D will be [0]:0x0A,[1]:0x0B,[2]:0x0C,[3]:0x0D
        blEnd = 'b';
    }
    else{
        goto errout;
    }

    //
    // Search tag number 0x8769 for offset of exif sub ifd
    //

    offset_numOfIfd0Entry = offset_tiffHeader + 8;
    numOfIfdEntry =  getBlEndInt32(blEnd, (u_int8_t *)&buf[offset_numOfIfd0Entry], 2);
    ifdEntry = (IfdEntry *) &buf[offset_numOfIfd0Entry + 2];

    for(i = 0; i < numOfIfdEntry; i++)
    {
        if(getBlEndInt32(blEnd, (u_int8_t *)&(ifdEntry->tagNum), 2) == 0x8769){
            // found Exif Sub IFD
            offset_exifSubIfd = getBlEndInt32(blEnd, (u_int8_t *)&(ifdEntry->data), 4);
            offset_exifSubIfd += offset_tiffHeader;
            break;
        }
        ifdEntry++;
    }
    if(i == numOfIfdEntry){
        goto errout;
    }

    //
    // Search tag number 0x9003 from exif sub ifd to get original date
    //

    numOfIfdEntry = getBlEndInt32(blEnd, (u_int8_t *)&(buf[offset_exifSubIfd]), 2);
    ifdEntry = (IfdEntry *)&buf[offset_exifSubIfd + 2];

    for(i = 0; i < numOfIfdEntry; i++)
    {
        if(getBlEndInt32(blEnd, (u_int8_t *)&(ifdEntry->tagNum), 2) == 0x9003){
            // original date
            offset_orgDate = getBlEndInt32(blEnd, (u_int8_t *)&(ifdEntry->data), 4);
            offset_orgDate += offset_tiffHeader;
            break;
        }
        ifdEntry++;
    }
    if(i == numOfIfdEntry){
        goto errout;
    }

    //
    // Get original date
    // format : YYYY:MM:DD
    //

    // year
    multiplier = 1000;
    for(i = 0; i < 4; i++){
        y += (buf[offset_orgDate + 0 + i] - 0x30) * multiplier;
        multiplier /= 10;
    }

    // month
    multiplier = 10;
    for(i = 0; i < 2; i++){
        m += (buf[offset_orgDate + 5 + i] - 0x30) * multiplier;
        multiplier /= 10;
    }

    // day
    multiplier = 10;
    for(i = 0; i < 2; i++){
        d += (buf[offset_orgDate + 8 + i] - 0x30) * multiplier;
        multiplier /= 10;
    }


errout:
    file.close();
    return QDate(y, m, d);
}

/*
 * name : getBlEndInt32
 * desc : Convet a serial of uint8 buffer to int as big/little endian,
 *        maximum length is 32bit
 * in   :
 *   bl, 'b' for big endian, 'l' for little endian
 *   *buf, pointer of buffer
 *   len, length of buffer
 * ret  : Convert result
 */
int PicIn_Core::getBlEndInt32(char bl, u_int8_t *buf, int len)
{
    int temp = 0;

    if(bl == 'b'){
        for(int i = 1; i <= len; i++){
            temp += buf[i - 1] << ((len - i) * 8);
        }
    }
    else if (bl == 'l'){
        for(int i = 0; i < len; i++){
            temp += buf[i] << i * 8;
        }
    }
    return temp;
}

/*
 * name : setFlagDir
 * desc : Set flag to decide whether separate pics to different dir as date
 * in   :
 *   year, Separate pics as year
 *   month, Separate pics as month
 *   day, Separate pics as day
 */
void PicIn_Core::setFlagDir(bool year, bool month, bool day)
{
    m_flagDirYear = year;
    m_flagDirMon = month;
    m_flagDirDay = day;
}

/*
 * name : setFlagCancel_true
 * desc : Set m_flagCancel to true
 */
void PicIn_Core::setFlagCancel_true(void)
{
    PicIn_Core::m_flagCancel = true;
}

/*
 * name : setFlagSubDir
 * desc : Set value of m_flagSubDir
 */
void PicIn_Core::setFlagSubDir(bool flag)
{
    PicIn_Core::m_flagSubDir = flag;
}

/*
 * name : setFlagOverwrite
 * desc : Set value of m_flagOverwrite
 */
void PicIn_Core::setFlagOverwrite(bool flag)
{
    PicIn_Core::m_flagOverwrite = flag;
}

/*
 * name : setFlagExifDate
 * desc : Set value of m_flagExifDate
 */
void PicIn_Core::setFlagExifDate(bool flag)
{
    PicIn_Core::m_flagExifDate = flag;
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

    if(m_flagSubDir){
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
    }

    return fileInfoList;
}

// ****************************************************************************
// ****                         Public Slots:                              ****
// ****************************************************************************
