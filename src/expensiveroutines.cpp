#include "expensiveroutines.h"
#include <QDebug>
#include "utils.h"



ExpensiveRoutines::ExpensiveRoutines()
{
    thread_ = new QThread();
    moveToThread(thread_);
    thread_->start();
}

ExpensiveRoutines::~ExpensiveRoutines()
{
    thread_->exit();
    thread_->wait();
    delete  thread_;
}

void ExpensiveRoutines::UpateProgress(double p)
{
    if ( static_cast<int>(p/max_iterations_) > p100_ ){
        p100_ = static_cast<int>( p/max_iterations_ );
        emit Progress(p100_);
    }
}

void ExpensiveRoutines::ResetProgress(int n)
{
    max_iterations_ = n*0.01;
    p100_=0;
}


void ExpensiveRoutines::export_annotations(AnnImgManager *info,
                                           AnnotationFilter filter,
                                           const QString &output_ann_path,
                                           int type_fmt)
{
    emit JobStarted(true);
    const QStringList lst = info->GetListOfImageIds();
    ResetProgress( lst.count() );

    size_t index_p=0;
    if (type_fmt==2){
        QFile file(output_ann_path);
        file.open(QFile::WriteOnly);
        QTextStream out(&file);
        for (const auto &img_id : lst){
            auto ann = filter.Filter( info->GetAnnotations(img_id) );
            QSize isize = info->GetImageSizeFromImage(img_id);
            out << img_id << "," << isize.width() << "," << isize.height();
            for (const auto &label : ann.keys() ){
                for (const auto &bbox : ann[label] ){
                    out << "," << label
                        << "," << bbox.left()
                        << "," << bbox.top()
                        << "," << bbox.right()
                        << "," << bbox.bottom();
                }
            }
            out << "\n";
            UpateProgress(++index_p);
        }
    } else if (type_fmt==0) {
        QDir ann_dir(output_ann_path);
        for (const auto &img_id : lst){
            auto ann = filter.Filter( info->GetAnnotations(img_id) );

            if ( !ann.isEmpty() ) {

                QFileInfo finfo(img_id);
                QFile ofile( ann_dir.filePath(finfo.baseName()+".csv") );
                ofile.open(QFile::WriteOnly);
                QTextStream out(&ofile);
                QSize isize = info->GetImageSizeFromImage(img_id);
                for (const auto &label : ann.keys() ){
                    for (const auto &bbox : ann[label] ){
                        out << img_id << "," << isize.width() << "," << isize.height()
                            << "," << label
                            << "," << bbox.left()
                            << "," << bbox.top()
                            << "," << bbox.right()
                            << "," << bbox.bottom() << "\n";
                    }
                }
                out.flush();
                ofile.close();
            }
            UpateProgress(++index_p);
        }
    } else if (type_fmt==1) {
        QDir ann_dir(output_ann_path);
        for (const auto &img_id : lst){
            QFileInfo finfo(img_id);
            auto ann = filter.Filter( info->GetAnnotations(img_id) );
            if ( !ann.isEmpty() ){
                Helper::SaveXMLSingleFile(ann_dir.filePath(finfo.baseName()+".xml"),
                                          ann,
                                          img_id,
                                          info->GetImageSizeFromImage(img_id));
            }
            UpateProgress(++index_p);
        }
    }
    JobFinished(false);
}
