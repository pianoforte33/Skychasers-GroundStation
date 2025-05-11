#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSerialPort>
#include <QString>
#include <QMessageBox>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QFile>

#define numberOfCharts 12
// position of the chart inside the QList chartList, seriesList, xList, yList, viewList and titleList
#define temperaturePosition 0
#define pressurePosition 1
#define humidityPosition 2
#define staticRadiationPosition 3
#define dynamicRadiationPosition 4
#define impulsesPosition 5
#define gyroscopePostition 6
#define anglePosition 7
#define accelerationPosition 8
#define gyroscopePostition2 9
#define anglePosition2 10
#define accelerationPosition2 11

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    QSerialPort* serial;

    bool autoScrollState = true;
    bool autoBackup = true;

    struct packet_struct {
        struct mpu6050Data_struct {
            // Temperature
            float temperature;
            // Angles
            float angX;
            float angY;
            float angZ;
            // Accelerometer
            int accX;
            int accY;
            int accZ;
            // Gyroscope
            int gyroX;
            int gyroY;
            int gyroZ;
        } mpu6050Data;

        struct mpu9250Data_struct {
            // Temperature
            float temperature;
            // Angles
            float angX;
            float angY;
            float angZ;
            // Accelerometer
            int accX;
            int accY;
            int accZ;
            // Gyroscope
            int gyroX;
            int gyroY;
            int gyroZ;
            // Mgnetometer
            int magX;
            int magY;
            int magZ;
        } mpu9250Data;

        struct bmpData_struct {
            float temperature;
            float pressure;
            float altitude;
            float verticalSpeed;
        } bmpData;

        struct bmeData_struct {
            float temperature;
            float pressure;
            float altitude;
            float humidity;
            float verticalSpeed;
        } bmeData;

        struct gpsData_struct {
            struct coordinates_struct {
                double lat;
                double lng;
            } coordinates;

            struct date_struct {
                uint16_t year;
                uint8_t month;
                uint8_t day;
            } date;

            struct time_struct {
                uint16_t hour;
                uint8_t minute;
                uint8_t second;
            } time;

            struct altitude_struct {
                float meters;
            } altitude;

            struct speed_struct {
                float mps;    // meters per second
            } speed;
        } gpsData;

        struct ccsData_struct {
            uint16_t co2;
            uint16_t voc;
        } ccsData;

        struct sgp41Data_struct {
            uint16_t voc;
            uint16_t nox;
        } sgp41;

        struct sgp30Data_struct {
            uint16_t co2;
            uint16_t voc;
            uint16_t rawH2;
            uint16_t rawEthanol;
        } sgp30;

        struct ensData_struct {
            float temperature;
            float humidity;
        } ens;

        struct shtData_struct {
            float temperature;
            float humidity;
        } sht;

        struct magData_struct {
            int magX;
            int magY;
            int magZ;
            int16_t azimuth;
        } magnetometer;

        struct radData_struct {
            uint32_t staticRadiation;
            uint32_t dynamicRadiation;
            uint16_t numberOfPulses;
            bool HVGeneratorState;
        } radsens;


        struct backsticksData_struct {
            bool motorStatus = false;   // DC motor
            uint16_t pidYP;
            uint16_t pidYN;
        } backsticks;

        int8_t mode = -1;
        /*
          * 1. Launch Pad Mode
          * 2. Ascent Mode
          * 3. Can Back Preparation Mode
          * 4. Can Back Mode
          * 5. Can Landing Mode
          * 6. Can Recovery Mode
        */

        int numberOfPacket;
        float voltage; // Battery
        float timeActive;
    };

    bool haveFirstHalf = false;
    packet_struct newPacket;
    QList<packet_struct> packetList; // storing the packet
    QString buffer; // storing all the data received

    void createChart(QChart*& chart, QLineSeries*& series, QValueAxis*& x, QValueAxis*& y, QChartView*& view, QString title);
    void destroyChart(QChart*& chart, QLineSeries*& series, QValueAxis*& x, QValueAxis*& y);

    //QChart *temperatureChart;
    //QLineSeries *seriesTemperature;
    //QValueAxis* axisXTemperature;
    //QValueAxis* axisYTemperature;

    QList<QChart*> chartList;
    QList<QLineSeries*> seriesList;
    QList<QValueAxis*> xList;
    QList<QValueAxis*> yList;
    QList<QChartView*> viewList;
    QList<QString> titleList;

    void adjustAxes(int index, float xValue, float yValue); // expand the chart
    void clearChart(); // delete all the points and lines in the chaarts

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void readData(); // read the data coming from the serial port
    void dividePacket(const QStringList fields, QChar marker); // divide the packet in the element of the struct
    void printPacket(); // print the packet on the serial monitor of the UI
    void updateUI(); // update the chart on the UI
    void writeToFile(); // write the packet on a file
    void printPacketConsole(const packet_struct &p); // debug

private slots:
    void on_buttonConnect_clicked();
    void on_buttonDisconnect_clicked();
    void on_actionEnable_Disable_autoScroll_triggered();
    void on_actionEnable_backup_changed();
    void on_actionClear_charts_triggered();
    void on_pushButton_send_clicked();
};
#endif // MAINWINDOW_H
