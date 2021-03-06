#include "picin_core.h"

PicIn_Core::PicIn_Core()
{
    m_numFiles = 0;
    m_options = 0;
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
    m_fileInfoList_src.clear();
    m_fileInfoList_src = get_file_list(m_pathList_source, filters);

    get_copyTgt_List();

    m_numFiles = m_fileInfoList_src.size();

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
 *    > 0, Which path isn't exist, 1 is 1st path
 *   -2, Invalid path type
 */
int PicIn_Core::set_path(QStringList pathList, PathType pt)
{
    if(pt != PT_Source && pt != PT_Target){
        return -2;
    }

    //
    // Check whether path has forward slash at last char
    //

    for(int i = 0; i < pathList.size(); i++){
        QString *path = (QString *)&pathList.at(i);
        QChar lastChar = path->at(path->size() - 1);;
        QString slash;

        slash.sprintf("%s", FILE_PATH_SEPARATOR);

        if(!operator ==(lastChar, slash.at(0))){
            path->append(slash);
        }

        //
        // Check path
        //

        QDir dir(*path);

        if(!dir.exists()){
            return i + 1;
        }
    }

    //
    // Set path
    //

    switch(pt){
    case PT_Source:
        m_pathList_source.clear();
        m_pathList_source.append(pathList);
        break;

    case PT_Target:
        m_pathList_target.clear();
        m_pathList_target.append(pathList);
        break;
    }

    return 0;
}

/*
 * name : setLastModifyDateTime
 * desc : Set indicated file's last modified time and last access time
 * in   :
 *   path, File's absolute path
 *   laDateTime, last access time
 *   lmDateTime, last modified time
 */
int PicIn_Core::setLastModifyDateTime(
    QString path,
    QDateTime laDateTime,
    QDateTime lmDateTime
)
{
#ifdef Q_OS_LINUX
        utimbuf timeBuf;

        timeBuf.actime = laDateTime.toTime_t();
        timeBuf.modtime = lmDateTime.toTime_t();

        return utime((const char*)path.toLatin1().data(), &timeBuf);
#else
    return 0;
#endif
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

    offOption(optionCancel);

    //If we don't do this proccessEvents(),
    //progress dialog would no response until this function done.
    QApplication::processEvents(QEventLoop::AllEvents);
    for(int i = 0; i < m_fileInfoList_src.size(); i++){
        srcPath = m_fileInfoList_src.at(i).absoluteFilePath();

        tgtPath.clear();
        tgtPath = m_copyTgtList.at(i);

        dir = QDir(QFileInfo(tgtPath).dir());
        if(!dir.exists()){
            dir.mkpath(dir.absolutePath());
        }

        //
        // Copy file
        //

        QDateTime laDateTime;
        QDateTime lmDateTime;

        laDateTime = QFileInfo(srcPath).lastRead();
        lmDateTime = QFileInfo(srcPath).lastModified();

        file.copy(srcPath, tgtPath);
        setLastModifyDateTime(tgtPath, laDateTime ,lmDateTime);

        //
        // Update progress bar and check cancel
        //

        emit signal_update_progress(i);
        if(checkOption(optionCancel)){
            break;
        }

        QApplication::processEvents(QEventLoop::AllEvents);
    }

    offOption(optionCancel);
}

/*
 * name : getExifDate
 * desc : get date time info from indicated file's exif
 */
QDate PicIn_Core::getExifDate(QString path)
{
#define JPG_SOI 0xffd8
#define JPG_SOS 0xffda
#define BUF_DEFAULT_SIZE 4096
    QFile file(path);
    int y = 0;
    int m = 0;
    int d = 0;

    char blEnd = 'l';

    u_int8_t *buf = NULL;

    u_int16_t offset_ffe1 = 0;
    u_int16_t offset_lenOfFfe1 = 0;
    u_int16_t app1Size = 0;

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
    int dateMultiplier = 0;

    int ret = 0;

    //
    // Start
    //

    buf = new u_int8_t[BUF_DEFAULT_SIZE];
    if(!buf){
        qWarning("allocate buffer failed\n");
        goto errout;
    }

    file.open(QIODevice::ReadOnly);
    ret = file.read((char *)buf, BUF_DEFAULT_SIZE);
    if(ret < 0){
        qWarning("Read %s failed\n", path.toLatin1().data());
    }

    //
    // Check 0xFFD8(Start Of Image)
    //

    if(getBlEndInt32('b', buf, 2) != JPG_SOI){
        qInfo("%s isn't jpg/jpeg\n", path.toLatin1().data());
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
    // Check if buffer size is enough
    //

    app1Size = getBlEndInt32('b', (u_int8_t *)&buf[offset_lenOfFfe1], 2);
    if(app1Size > BUF_DEFAULT_SIZE){
        delete[] buf;
        buf = new u_int8_t[app1Size];
        if(!buf){
            qWarning("Can't re-assign buffer\n");
            goto errout;
        }
        ret = file.seek(0);
        ret = file.read((char *)buf, app1Size);
        if(ret < 0 || ret < app1Size){
            qWarning("Read %s to new buffer failed\n", path.toLatin1().data());
            goto errout;
        }
    }

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
        qWarning("%s can't find exif sub ifd\n", path.toLatin1().data());
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
        qWarning("%s can't find original date from exif\n", path.toLatin1().data());
        goto errout;
    }

    //
    // Get original date
    // format : YYYY:MM:DD
    //

    // year
    dateMultiplier = 1000;
    for(i = 0; i < 4; i++){
        y += (buf[offset_orgDate + 0 + i] - 0x30) * dateMultiplier;
        dateMultiplier /= 10;
    }

    // month
    dateMultiplier = 10;
    for(i = 0; i < 2; i++){
        m += (buf[offset_orgDate + 5 + i] - 0x30) * dateMultiplier;
        dateMultiplier /= 10;
    }

    // day
    dateMultiplier = 10;
    for(i = 0; i < 2; i++){
        d += (buf[offset_orgDate + 8 + i] - 0x30) * dateMultiplier;
        dateMultiplier /= 10;
    }


errout:
    file.close();
    if(buf){
        delete[] buf;
    }
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
 * name : setOptions
 * desc : Set m_options value
 * in   :
 *   options, Indicated options value
 */
void PicIn_Core::setOption(Options options)
{
    m_options = options;
}

/*
 * name : getOptions
 * desc : Get current options
 * ret  :
 *   Options, Current options
 */
PicIn_Core::Options PicIn_Core::getOptions(void)
{
    return m_options;
}

/*
 * name : onOption
 * desc : Enable specific option
 * in   :
 *   on, Option to disable
 */
void PicIn_Core::onOption(Options on)
{
    m_options |= on;
    return;
}

/*
 * name : offOption
 * desc : Disable specific option
 * in   :
 *   off, Option to disable
 */
void PicIn_Core::offOption(Options off)
{
    m_options &= ~off;
}

/*
 * name : checkOption
 * desc : Check specific option has been set or not
 * in   :
 *   check, Option to check
 * ret  :
 *   true, if option has been set
 *   false, Option is un-set
 */
bool PicIn_Core::checkOption(Options check)
{
    return m_options & check;
}

// ****************************************************************************
// ****                      Private functions:                            ****
// ****************************************************************************


/*
 * name : get_file_list
 * desc : Scan indicated path list and return file info list of images
 * in   :
 *   pathList, String list of source path
 *   filters, String list of file extension name
 * ret  : List of QFileInfo for all files under indicated path
 */
QFileInfoList
PicIn_Core::get_file_list(QStringList pathList, QStringList filters)
{
    QFileInfoList fileInfoList;
    for(int i = 0; i < pathList.size(); i++){
        fileInfoList.append(get_file_list(pathList.at(i), filters));
    }

    return fileInfoList;
}

/*
 * name : get_file_list
 * desc : Scan indicated path and return file info list of images
 * in   :
 *   path, String of source path
 *   filters, String list of file extension name
 * ret  : List of QFileInfo for all files under indicated path
 */
QFileInfoList
PicIn_Core::get_file_list(QString path, QStringList filters)
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

    if(checkOption(optionSubDir)){
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

/*
 * name : get_copyTgt_List
 * desc : Scan whole source list and target path,
 *        if path has existed and optionOverwrite is off,
 *        the source path would removed from list
 */
void PicIn_Core::get_copyTgt_List(void)
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
    QFileInfoList tempSrcList;

    tempSrcList.clear();
    m_copyTgtList.clear();

    for(int i = 0; i < m_fileInfoList_src.size(); i++){
        srcPath = m_fileInfoList_src.at(i).absoluteFilePath();

        tgtName.clear();
        tgtName.append(m_fileInfoList_src.at(i).fileName());

        tgtPath.clear();
        tgtPath.append(m_pathList_target.at(0)); // Only support 1 target path

        //
        // Check whether need to separate pics to folders as date
        //

        if(checkOption(optionExifDate)){
            date = getExifDate(srcPath);
        }
        if(!date.isValid() || !checkOption(optionExifDate)){
            date = m_fileInfoList_src.at(i).lastModified().date();
        }

        yearPath.clear();
        monthPath.clear();
        dayPath.clear();
        if(checkOption(optionDirYear)){
            yearPath.sprintf("/%04d/", date.year());
            tgtPath.append(yearPath);
        }
        if(checkOption(optionDirMon)){
            monthPath.sprintf("/%02d/", date.month());
            tgtPath.append(monthPath);
        }
        if(checkOption(optionDirDay)){
            dayPath.sprintf("/%02d/", date.day());
            tgtPath.append(dayPath);
        }

        //
        // Check whether file need to be copied
        //

        tgtPath.append(tgtName);
        if(!file.exists(tgtPath) || checkOption(optionOverwrite)){
            m_copyTgtList.append(tgtPath);
            tempSrcList.append(m_fileInfoList_src.at(i));
        }
    }

    m_fileInfoList_src = tempSrcList;

    return;
}

// ****************************************************************************
// ****                         Public Slots:                              ****
// ****************************************************************************
