// SPDX-FileCopyrightText: 2014-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOARDDIALOG_H_
#define BOARDDIALOG_H_

#include <QFileDialog>

class QLabel;

class BoardDialog : public QFileDialog {
  Q_OBJECT

 public:
  explicit BoardDialog(QWidget *pParent, const QString &sCaption = QString(),
                       const QString &sDirectory = QString(),
                       const QString &sFilter = QString());

 protected slots:
  void OnCurrentChanged(const QString &sPath);

 private:
  QLabel *m_pSolutions;
  QLabel *m_pPreviewCaption;
  QLabel *m_pPreview;
  const QSize previewsize;
};

#endif  // BOARDDIALOG_H_
