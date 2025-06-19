#ifndef WORKERLIST_H
#define WORKERLIST_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QTableWidget>          
#include <QTableWidgetItem>  

class WorkerListPage : public QWidget {
public:
    explicit WorkerListPage(QWidget *parent = nullptr);
    void loadWorkers();

private:
    QTableWidget *table;
    QPushButton *addWorkerBtn;
    QPushButton *removeWorkerBtn;
};

#endif // WORKERLIST_H
