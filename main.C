#include <QCoreApplication>
#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>
#include <iostream>

void update_table(int shipper_zip, int low_zip, int high_zip, int z_ground, int  z_2DA,
                  QSqlTableModel &out_model, QSqlRecord &out_record) {
//    qDebug() << low_zip << high_zip << z_ground << z_2DA << endl;
    out_record.setValue("z_ground", z_ground);
    out_record.setGenerated("z_ground",true);
    out_record.setValue("z_2DA", z_2DA);
    out_record.setGenerated("z_2DA",true);
    out_record.setValue("shipper_zip", shipper_zip);
    out_record.setGenerated("shipper_zip",true);
    out_record.setValue("low_zip", low_zip);
    out_record.setGenerated("low_zip",true);
    out_record.setValue("high_zip", high_zip);
    out_record.setGenerated("high_zip",true);
    if (!out_model.insertRecord(-1,out_record)) {
        std::cout << "Database Write Error" << " The database reported an error: " <<
                     out_model.lastError().text().toStdString();
    } /* endif */
    out_model.submit();
} /* update_table */

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("kaaker");
    db.setUserName(getenv("USER"));
    db.setPassword("var6look");
    bool ok = db.open();
    if (!ok) {

    }

    QSqlTableModel in_model;
    in_model.setTable("UPS_ZIPS_to_GND_2DA_zones");
    in_model.select();
    QSqlTableModel out_model;
    out_model.setTable("UPS_condensed_zips_to_GND_2DA_zones");
//    out_model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    QSqlRecord in_record = in_model.record();
    QSqlRecord out_record = out_model.record();

    int samples_count = 0;
    int low_zip = in_model.record(0).value("dest_ZIP").toInt();
    int high_zip = low_zip;
    int last_ground_zone = in_model.record(0).value("z_ground").toInt();
    int last_z_2DA = in_model.record(0).value("z_2DA").toInt();
    int last_shipper_zip = in_model.record(0).value("shipper_zip").toInt();

    for (int i = 0; i < in_model.rowCount(); ++i) {
        in_record = in_model.record(i);
        int dest_zip = in_record.value("dest_zip").toInt();
        int shipper_zip = in_record.value("shipper_zip").toInt();
        int z_ground = in_record.value("z_ground").toInt();
        int z_2DA = in_record.value("z_2DA").toInt();
       qDebug() << "row " << i << low_zip << high_zip << z_ground << z_2DA << endl;
        if (shipper_zip != last_shipper_zip) { /* Reached a zone boundary, make a record and reset */
            if (samples_count > 0) {
                update_table(last_shipper_zip, low_zip, high_zip, last_ground_zone,
                             last_z_2DA,
                             out_model, out_record);
            } /* endif */
            samples_count = 0;
            last_shipper_zip = shipper_zip;
            last_ground_zone = z_ground;
            last_z_2DA = z_2DA;
            low_zip = dest_zip;
            high_zip = dest_zip;
        } /* endif */
        ++samples_count;
        if (((z_ground != last_ground_zone) || (z_2DA != last_z_2DA))) { /* Reached a zone boundary, reset the accumulation */
            if (samples_count > 0) {
                update_table(last_shipper_zip, low_zip, high_zip, last_ground_zone,
                             last_z_2DA,
                             out_model, out_record);
            } /* endif */
            samples_count = 0;
            last_shipper_zip = shipper_zip;
            last_ground_zone = z_ground;
            last_z_2DA = z_2DA;
            low_zip = dest_zip;
            samples_count = 0;
        } /* endif */
        high_zip = dest_zip;
    } /* endfor */
    if (samples_count > 0) {
        update_table(last_shipper_zip, low_zip, high_zip, last_ground_zone,
                     last_z_2DA,
                     out_model, out_record);
    } /* endif */
    if (!out_model.submitAll()) {
       std::cout << "Database Write Error" << " The database reported an error: " <<
                    out_model.lastError().text().toStdString() << std::endl;
    } /* endif */

    //return a.exec();
    return 0;
}
