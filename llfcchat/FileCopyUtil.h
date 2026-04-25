#ifndef FILECOPYUTIL_H
#define FILECOPYUTIL_H

#include <QFile>
#include <QDir>
#include <QCoreApplication>

class FileCopyUtil
{
public:
    // 拷贝单个文件
    static bool copyFile(const QString& srcPath, const QString& destPath)
    {
        QFile srcFile(srcPath);
        if (!srcFile.exists()) return false;

        QFile destFile(destPath);
        if (destFile.exists()) destFile.remove();

        return srcFile.copy(destPath);
    }

    // 递归拷贝目录
    static bool copyDir(const QString& srcDirPath, const QString& destDirPath)
    {
        QDir srcDir(srcDirPath);
        if (!srcDir.exists()) return false;

        QDir destDir(destDirPath);
        if (!destDir.exists()) destDir.mkpath(destDirPath);

        // 拷贝文件
        for (const QFileInfo& file : srcDir.entryInfoList(QDir::Files)) {
            if (!copyFile(file.absoluteFilePath(), destDir.absoluteFilePath(file.fileName())))
                return false;
        }

        // 递归拷贝子目录
        for (const QFileInfo& dir : srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (!copyDir(dir.absoluteFilePath(), destDir.absoluteFilePath(dir.fileName())))
                return false;
        }

        return true;
    }
};

#endif // FILECOPYUTIL_H
