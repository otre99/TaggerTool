#include "recentprojects.h"

#include <QCryptographicHash>
#include <QSettings>

#include "utils.h"
constexpr int MAX_ENTRIES = 32;

RecentProjects::RecentProjects() {
  QSettings st;
  st.beginGroup("recent_image_annotation_folders");

  m_sortedKeys = st.childKeys();
  for (auto &&key : m_sortedKeys) {
    RecentEntry e;
    auto value = st.value(key);
    unpackProjectTuple(value, e.imgFolder, e.annFolder, e.secondsToYear5000);
    m_recents[key] = e;
  }
  st.endGroup();
  sortKeys();
}

RecentProjects::~RecentProjects() {
  QSettings st;
  st.beginGroup("recent_image_annotation_folders");
  st.remove("");
  auto N = qMin(m_recents.size(), MAX_ENTRIES);
  for (int i = 0; i < N; ++i) {
    QString key = m_sortedKeys[i];
    const RecentEntry &e = m_recents[key];
    auto value =
        packProjectTuple(e.imgFolder, e.annFolder, e.secondsToYear5000);
    st.setValue(key, value);
  }
  st.endGroup();
}

QList<RecentProjects::RecentEntry> RecentProjects::recentEntries() const {
  auto N = qMin(m_recents.size(), MAX_ENTRIES);
  QList<RecentProjects::RecentEntry> result(N);
  for (int i = 0; i < N; ++i) {
    result[i] = m_recents[m_sortedKeys[i]];
  }
  return result;
}

void RecentProjects::updateEntry(const QString &imgFolder,
                                 const QString &annFolder,
                                 const std::optional<QDateTime> &dt) {
  const QString key = makeProjectKey(imgFolder, annFolder);
  if (!m_recents.contains(key)) {
    m_sortedKeys.push_front(key);
    m_recents[key] =
        RecentEntry{imgFolder, annFolder, Helper::seconsToYear5000(dt)};
  } else {
    m_recents[key].secondsToYear5000 = Helper::seconsToYear5000(dt);
    sortKeys();
  }
}

// aux functions
void RecentProjects::sortKeys() {
  std::sort(m_sortedKeys.begin(), m_sortedKeys.end(),
            [this](auto &a, auto &b) { return m_recents[a] <= m_recents[b]; });
}

inline QString RecentProjects::makeProjectKey(const QString &image,
                                              const QString &anno) {
  const auto h = QCryptographicHash::hash((image + u'-' + anno).toUtf8(),
                                          QCryptographicHash::Sha1);
  return QString::fromLatin1(
      h.toHex());  // store mapping from hash->packed tuple
}

inline QVariant RecentProjects::packProjectTuple(const QString &image,
                                                 const QString &anno,
                                                 qint64 secs) {
  QVariantList v;
  v << image << anno << QVariant::fromValue(secs);
  return v;
}

inline bool RecentProjects::unpackProjectTuple(const QVariant &v,
                                               QString &image, QString &anno,
                                               qint64 &secs) {
  const auto lst = v.toList();
  if (lst.size() != 3) return false;
  image = lst[0].toString();
  anno = lst[1].toString();
  secs = lst[2].toLongLong();
  return true;
}
