#ifndef RECENTPROJECTS_H
#define RECENTPROJECTS_H
#include <QDateTime>
#include <QMap>
#include <QString>
class RecentProjects {
 public:
  struct RecentEntry {
    QString imgFolder;
    QString annFolder;
    qint64 secondsToYear5000;
    // Must be a strict weak ordering: std::sort has undefined behaviour with a
    // comparator that returns true for equal elements.
    bool operator<(const RecentEntry &other) const {
      return this->secondsToYear5000 < other.secondsToYear5000;
    }
  };
  RecentProjects();
  QList<RecentEntry> recentEntries() const;
  void updateEntry(const QString &imgFolder, const QString &annFolder,
                   const std::optional<QDateTime> &dt = std::nullopt);
  ~RecentProjects();

 private:
  void sortKeys();
  QMap<QString, RecentEntry> m_recents;
  QStringList m_sortedKeys;

  // aux functions
  inline QString makeProjectKey(const QString &image, const QString &anno);
  inline QVariant packProjectTuple(const QString &image, const QString &anno,
                                   qint64 secs);
  inline bool unpackProjectTuple(const QVariant &v, QString &image,
                                 QString &anno, qint64 &secs);
};

#endif  // RECENTPROJECTS_H
