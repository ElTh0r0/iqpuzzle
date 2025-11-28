// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOARDPREVIEW_H_
#define BOARDPREVIEW_H_

#include <QMouseEvent>
#include <QWidget>

namespace Ui {
class BoardPreview;
}

class BoardPreview : public QWidget {
  Q_OBJECT

 public:
  explicit BoardPreview(const QString &sFilePath, const bool bSolved,
                        const QSize previewsize, QWidget *pParent = nullptr);
  ~BoardPreview();

  auto getName() -> const QString;
  void updateSolved();

 public slots:
  void updateUiLang();

 signals:
  void selectBoard(const QString &sFileName);

 protected:
  void mousePressEvent(QMouseEvent *p_Event) override;

 private:
  void setSolutions(QString const &sFilePath);
  void setPreview(QString sFilePath, const QSize previewsize);

  Ui::BoardPreview *m_pUi;
  QString m_sFilePath;
  quint32 m_nSolutions;
};

#endif  // BOARDPREVIEW_H_
