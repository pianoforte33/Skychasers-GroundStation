/*
 * first half of the packet:
    @(float)mpu6050Data.temperature;(float)mpu6050Data.angX;(float)mpu6050Data.angY;(float)mpu6050Data.angZ;(int)mpu6050Data.accX;(int)mpu6050Data.accY;(int)mpu6050Data.accZ;(int)mpu6050Data.gyroX;(int)mpu6050Data.gyroY;(int)mpu6050Data.gyroZ;(float)mpu9250Data.temperature;(float)mpu9250Data.angX;(float)mpu9250Data.angY;(float)mpu9250Data.angZ;(int)mpu9250Data.accX;(int)mpu9250Data.accY;(int)mpu9250Data.accZ;(int)mpu9250Data.gyroX;(int)mpu9250Data.gyroY;(int)mpu9250Data.gyroZ;(int)mpu9250Data.magX;(int)mpu9250Data.magY;(int)mpu9250Data.magZ;(float)bmpData.temperature;(float)bmpData.pressure;(float)bmpData.altitude;(float)bmpData.verticalSpeed;(float)bmeData.temperature;(float)bmeData.pressure;(float)bmeData.altitude;(float)bmeData.humidity;(float)bmeData.verticalSpeed;(double)gpsData.coordinates.lat;(double)gpsData.coordinates.lng;(uint16_t)gpsData.date.year;(uint8_t)gpsData.date.month;(uint8_t)gpsData.date.day;
 * second half of the packet
    &(uint16_t)gpsData.time.hour;(uint8_t)gpsData.time.minute;(uint8_t)gpsData.time.second;(float)gpsData.altitude.meters;(float)gpsData.speed.mps;(uint16_t)ccsData.co2;(uint16_t)ccsData.voc;(uint16_t)sgp41.voc;(uint16_t)sgp41.nox;(uint16_t)sgp30.co2;(uint16_t)sgp30.voc;(uint16_t)sgp30.rawH2;(uint16_t)sgp30.rawEthanol;(float)ens.temperature;(float)ens.humidity;(float)sht.temperature;(float)sht.humidity;(int)magnetometer.magX;(int)magnetometer.magY;(int)magnetometer.magZ;(int16_t)magnetometer.azimuth;(uint32_t)radsens.staticRadiation;(uint32_t)radsens.dynamicRadiation;(uint16_t)radsens.numberOfPulses;(bool)radsens.HVGeneratorState;(bool)backsticks.motorStatus;(uint16_t)backsticks.pidYP;(uint16_t)backsticks.pidYN;(int16_t)LoRaData.rssi;(int8_t)mode;(float)voltage;(float)timeActive;(uint32_t);
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial(new QSerialPort(this))
{
    ui->setupUi(this);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    // adding all the option for setting the baud rate in the combo box
    ui->setBaudRate->addItem("1200");
    ui->setBaudRate->addItem("2400");
    ui->setBaudRate->addItem("4800");
    ui->setBaudRate->addItem("9600");
    ui->setBaudRate->addItem("19200");
    ui->setBaudRate->addItem("38400");
    ui->setBaudRate->addItem("57600");
    ui->setBaudRate->addItem("115200");
    ui->setBaudRate->addItem("921600");
    ui->setBaudRate->setCurrentIndex(3); // set as the default 9600;
    ui->setPortName->setText("/dev/ttyACM0"); //for debug

    chartList.reserve(numberOfCharts);
    chartList.resize(numberOfCharts);
    seriesList.reserve(numberOfCharts);
    seriesList.resize(numberOfCharts);
    xList.reserve(numberOfCharts);
    xList.resize(numberOfCharts);
    yList.reserve(numberOfCharts);
    yList.resize(numberOfCharts);
    titleList.reserve(numberOfCharts);
    titleList.resize(numberOfCharts);
    viewList.reserve(numberOfCharts);
    viewList.resize(numberOfCharts);
    viewList = {  ui->temperature_chart
                , ui->pressure_chart
                , ui->humidity_chart
                , ui->staticRadiation_chart
                , ui->dynamicRadiation_chart
                , ui->impulses_chart
                , ui->gyroscope_chart
                , ui->angle_chart
                , ui->acceleration_chart
                , ui->gyroscope_chart_2
                , ui->angle_chart_2
                , ui->acceleration_chart_2
    };

    titleList = { "Temperature"
                , "Pressure"
                , "Humidity"
                , "Static radiation"
                , "Dynamic radiation"
                , "Number of impulses"
                , "Gyroscope"
                , "Angle"
                , "Acceleration"
                , "Gyroscope"
                , "Angle"
                , "Acceleration"
    };

    for (int i = 0; i < numberOfCharts; i++) {
        createChart(chartList[i], seriesList[i], xList[i], yList[i], viewList[i], titleList[i]);
    }
}

void MainWindow::createChart(QChart*& chart, QLineSeries*& series, QValueAxis*& x, QValueAxis*& y, QChartView*& view, QString title) {
    series = new QLineSeries;
    chart = new QChart;

    x = new QValueAxis;
    y = new QValueAxis;

    chart->legend()->hide();
    chart->addSeries(series);
    chart->setBackgroundVisible(false);
    chart->setTheme(QChart::ChartThemeDark);
    chart->setMargins(QMargins(0,0,0,0));
    chart->setTitle(title);

    x->setRange(0, 100);
    y->setRange(0, 100);
    chart->addAxis(x, Qt::AlignBottom);
    chart->addAxis(y, Qt::AlignLeft);
    series->attachAxis(x);
    series->attachAxis(y);

    view->setRenderHint(QPainter::Antialiasing);
    view->setChart(chart);
}

void MainWindow::on_actionEnable_Disable_autoScroll_triggered() {
    autoScrollState = !autoScrollState;
}

void MainWindow::on_actionEnable_backup_changed()
{
    autoBackup = !autoBackup;
}

void MainWindow::on_buttonConnect_clicked() {
    serial->setPortName(ui->setPortName->text());
    serial->setBaudRate(ui->setBaudRate->currentText().toInt());

    if (!serial->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, "ERROR", "failed to open the serial port: " + serial->errorString());
    }
}

void MainWindow::on_buttonDisconnect_clicked() {
    if (serial->isOpen()) {
        serial->close();
    }
}

void MainWindow::readData() {
    buffer.append(serial->readAll());
    printPacket();

    int posAt = buffer.indexOf('@');
    int posAmp = buffer.indexOf('&');
    int posStart;
    int posEnd;
    QChar marker;
    QString packet;
    QStringList fields;

    if (posAt == -1 && posAmp == -1) {
        return;
    }

    if (posAt != -1 && (posAmp == -1 || posAt < posAmp)) {
        posStart = posAt;
        marker   = '@';
    } else {
        posStart = posAmp;
        marker   = '&';
    }

    posEnd = buffer.indexOf('\r', posStart);
    if (posEnd == -1) {
        return;
    }
    packet = buffer.mid(posStart + 1, posEnd - posStart - 1);
    packet = packet.trimmed();
    fields = packet.split(';', Qt::SkipEmptyParts);
    buffer.remove(0, posEnd + 1);

    printPacket();
    dividePacket(fields, marker);
}

void MainWindow::dividePacket(QStringList fields, QChar marker) {
    int index = 0;

    if (marker == '@') {
        if (fields.size() != 37) {
            return;
        }
        // mpu6050Data
        newPacket.mpu6050Data.temperature = fields[index++].toFloat();
        newPacket.mpu6050Data.angX = fields[index++].toFloat();
        newPacket.mpu6050Data.angY = fields[index++].toFloat();
        newPacket.mpu6050Data.angZ = fields[index++].toFloat();
        newPacket.mpu6050Data.accX = fields[index++].toInt();
        newPacket.mpu6050Data.accY = fields[index++].toInt();
        newPacket.mpu6050Data.accZ = fields[index++].toInt();
        newPacket.mpu6050Data.gyroX = fields[index++].toInt();
        newPacket.mpu6050Data.gyroY = fields[index++].toInt();
        newPacket.mpu6050Data.gyroZ = fields[index++].toInt();
        // mpu9250Data
        newPacket.mpu9250Data.temperature = fields[index++].toFloat();
        newPacket.mpu9250Data.angX = fields[index++].toFloat();
        newPacket.mpu9250Data.angY = fields[index++].toFloat();
        newPacket.mpu9250Data.angZ = fields[index++].toFloat();
        newPacket.mpu9250Data.accX = fields[index++].toInt();
        newPacket.mpu9250Data.accY = fields[index++].toInt();
        newPacket.mpu9250Data.accZ = fields[index++].toInt();
        newPacket.mpu9250Data.gyroX = fields[index++].toInt();
        newPacket.mpu9250Data.gyroY = fields[index++].toInt();
        newPacket.mpu9250Data.gyroZ = fields[index++].toInt();
        newPacket.mpu9250Data.magX = fields[index++].toInt();
        newPacket.mpu9250Data.magY = fields[index++].toInt();
        newPacket.mpu9250Data.magZ = fields[index++].toInt();
        // bmpData & bmeData
        newPacket.bmpData.temperature = fields[index++].toFloat();
        newPacket.bmpData.pressure = fields[index++].toFloat();
        newPacket.bmpData.altitude = fields[index++].toFloat();
        newPacket.bmpData.verticalSpeed = fields[index++].toFloat();
        newPacket.bmeData.temperature = fields[index++].toFloat();
        newPacket.bmeData.pressure = fields[index++].toFloat();
        newPacket.bmeData.altitude = fields[index++].toFloat();
        newPacket.bmeData.humidity = fields[index++].toFloat();
        newPacket.bmeData.verticalSpeed = fields[index++].toFloat();
        // GPS date and coordinates
        newPacket.gpsData.coordinates.lat = fields[index++].toDouble();
        newPacket.gpsData.coordinates.lng = fields[index++].toDouble();
        newPacket.gpsData.date.year = fields[index++].toUShort();
        newPacket.gpsData.date.month = fields[index++].toUShort();
        newPacket.gpsData.date.day = fields[index++].toUShort();

        haveFirstHalf = true;
    } else {
        if (fields.size() != 32 || !haveFirstHalf) {
            return;
        }
        // GPS time, altitude, speed
        newPacket.gpsData.time.hour = fields[index++].toUShort();
        newPacket.gpsData.time.minute = fields[index++].toUShort();
        newPacket.gpsData.time.second = fields[index++].toUShort();
        newPacket.gpsData.altitude.meters = fields[index++].toFloat();
        newPacket.gpsData.speed.mps = fields[index++].toFloat();
        // gas sensors
        newPacket.ccsData.co2 = fields[index++].toUShort();
        newPacket.ccsData.voc = fields[index++].toUShort();
        newPacket.sgp41.voc = fields[index++].toUShort();
        newPacket.sgp41.nox = fields[index++].toUShort();
        newPacket.sgp30.co2 = fields[index++].toUShort();
        newPacket.sgp30.voc = fields[index++].toUShort();
        newPacket.sgp30.rawH2 = fields[index++].toUShort();
        newPacket.sgp30.rawEthanol = fields[index++].toUShort();
        // environmental sensors
        newPacket.ens.temperature = fields[index++].toFloat();
        newPacket.ens.humidity = fields[index++].toFloat();
        newPacket.sht.temperature = fields[index++].toFloat();
        newPacket.sht.humidity = fields[index++].toFloat();
        // magnetometer & azimuth
        newPacket.magnetometer.magX = fields[index++].toInt();
        newPacket.magnetometer.magY = fields[index++].toInt();
        newPacket.magnetometer.magZ = fields[index++].toInt();
        newPacket.magnetometer.azimuth = fields[index++].toInt();
        // radiation data
        newPacket.radsens.staticRadiation = fields[index++].toUInt();
        newPacket.radsens.dynamicRadiation = fields[index++].toUInt();
        newPacket.radsens.numberOfPulses = fields[index++].toUShort();
        newPacket.radsens.HVGeneratorState = (fields[index++].toInt() != 0);
        // backsticks
        newPacket.backsticks.motorStatus = (fields[index++].toInt() != 0);
        newPacket.backsticks.pidYP = fields[index++].toUShort();
        newPacket.backsticks.pidYN = fields[index++].toUShort();
        // remaining top‐level fields
        newPacket.mode = fields[index++].toInt();
        newPacket.numberOfPacket = fields[index++].toInt();
        newPacket.voltage = fields[index++].toFloat();
        newPacket.timeActive = fields[index++].toFloat();

        haveFirstHalf = false;
        packetList.push_back(newPacket);
        updateUI();
        writeToFile();
    }
}

void MainWindow::writeToFile () {
    bool fileExists = QFile::exists("backup.csv");
    QFile file("backup.csv");

    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
        return;
    }

    QTextStream out(&file);

    if (!fileExists) {
        out << "mpu6050_temp;mpu6050_angX;mpu6050_angY;mpu6050_angZ;mpu6050_accX;mpu6050_accY;mpu6050_accZ;mpu6050_gyroX;mpu6050_gyroY;mpu6050_gyroZ;mpu9250_temp;mpu9250_angX;mpu9250_angY;mpu9250_angZ;mpu9250_accX;mpu9250_accY;mpu9250_accZ;mpu9250_gyroX;mpu9250_gyroY;mpu9250_gyroZ;mpu9250_magX;mpu9250_magY;mpu9250_magZ;bmp_temp;bmp_pressure;bmp_altitude;bmp_vertSpeed;bme_temp;bme_pressure;bme_altitude;bme_humidity;bme_vertSpeed;gps_lat;gps_lng;gps_date_year;gps_date_month;gps_date_day;gps_time_hour;gps_time_minute;gps_time_second;gps_altitude_meters;gps_speed_mps;ccs_co2;ccs_voc;sgp41_voc;sgp41_nox;gp30_co2;sgp30_voc;sgp30_rawH2;sgp30_rawEthanol; ens_temp;ens_humidity;sht_temp;sht_humidity;magX;magY;magZ;mag_azimuth;rad_static;rad_dynamic;rad_pulses;rad_HVstate;back_motorStatus;back_pidYP;back_pidYN;mode;numberOfPacket;voltage;timeActive\n";
    }

    if (packetList.isEmpty()) {
        return;
    }

    packet_struct p = packetList.back();
    out
        // mpu6050Data
        << p.mpu6050Data.temperature << ';'
        << p.mpu6050Data.angX << ';'
        << p.mpu6050Data.angY << ';'
        << p.mpu6050Data.angZ << ';'
        << p.mpu6050Data.accX << ';'
        << p.mpu6050Data.accY << ';'
        << p.mpu6050Data.accZ << ';'
        << p.mpu6050Data.gyroX << ';'
        << p.mpu6050Data.gyroY << ';'
        << p.mpu6050Data.gyroZ << ';'

        // mpu9250Data
        << p.mpu9250Data.temperature << ';'
        << p.mpu9250Data.angX << ';'
        << p.mpu9250Data.angY << ';'
        << p.mpu9250Data.angZ << ';'
        << p.mpu9250Data.accX << ';'
        << p.mpu9250Data.accY << ';'
        << p.mpu9250Data.accZ << ';'
        << p.mpu9250Data.gyroX << ';'
        << p.mpu9250Data.gyroY << ';'
        << p.mpu9250Data.gyroZ << ';'
        << p.mpu9250Data.magX << ';'
        << p.mpu9250Data.magY << ';'
        << p.mpu9250Data.magZ << ';'

        // bmpData
        << p.bmpData.temperature << ';'
        << p.bmpData.pressure << ';'
        << p.bmpData.altitude << ';'
        << p.bmpData.verticalSpeed << ';'

        // bmeData
        << p.bmeData.temperature << ';'
        << p.bmeData.pressure << ';'
        << p.bmeData.altitude << ';'
        << p.bmeData.humidity << ';'
        << p.bmeData.verticalSpeed << ';'

        // gpsData
        << p.gpsData.coordinates.lat << ';'
        << p.gpsData.coordinates.lng << ';'
        << p.gpsData.date.year << ';'
        << static_cast<int>(p.gpsData.date.month) << ';'
        << static_cast<int>(p.gpsData.date.day) << ';'
        << p.gpsData.time.hour << ';'
        << static_cast<int>(p.gpsData.time.minute) << ';'
        << static_cast<int>(p.gpsData.time.second) << ';'
        << p.gpsData.altitude.meters << ';'
        << p.gpsData.speed.mps << ';'

        // ccsData
        << p.ccsData.co2 << ';'
        << p.ccsData.voc << ';'

        // sgp41
        << p.sgp41.voc << ';'
        << p.sgp41.nox << ';'

        // sgp30
        << p.sgp30.co2 << ';'
        << p.sgp30.voc << ';'
        << p.sgp30.rawH2 << ';'
        << p.sgp30.rawEthanol << ';'

        // ens
        << p.ens.temperature << ';'
        << p.ens.humidity << ';'

        // sht
        << p.sht.temperature << ';'
        << p.sht.humidity << ';'

        // magnetometer
        << p.magnetometer.magX << ';'
        << p.magnetometer.magY << ';'
        << p.magnetometer.magZ << ';'
        << p.magnetometer.azimuth << ';'

        // radsens
        << p.radsens.staticRadiation << ';'
        << p.radsens.dynamicRadiation << ';'
        << p.radsens.numberOfPulses << ';'
        << (p.radsens.HVGeneratorState ? 1 : 0) << ';'

        // backsticks
        << (p.backsticks.motorStatus ? 1 : 0) << ';'
        << p.backsticks.pidYP << ';'
        << p.backsticks.pidYN << ';'

        // remaining top‐level fields
        << static_cast<int>(p.mode) << ';'
        << p.numberOfPacket << ';'
        << p.voltage << ';'
        << p.timeActive << ';'
        << '\n';
}

void MainWindow::updateUI() {
    const auto &p = packetList.back();
    float t = p.timeActive;

    // Temperature (average of 6 sensors)
    float tempAverage = (p.mpu6050Data.temperature
                         + p.mpu9250Data.temperature
                         + p.bmeData.temperature
                         + p.bmpData.temperature
                         + p.ens.temperature
                         + p.sht.temperature) / (float)6;
    seriesList[temperaturePosition]->append(t, tempAverage);
    adjustAxes(temperaturePosition, t, tempAverage);

    // Pressure (average of BMP + BME)
    float pressureAverage = (p.bmpData.pressure + p.bmeData.pressure) / 2.0f;
    seriesList[pressurePosition]->append(t, pressureAverage);
    adjustAxes(pressurePosition, t, pressureAverage);

    // Humidity (average of BME + ENS + SHT)
    float humidityAverage = (p.bmeData.humidity + p.ens.humidity + p.sht.humidity) / 3.0f;
    seriesList[humidityPosition]->append(t, humidityAverage);
    adjustAxes(humidityPosition, t, humidityAverage);

    // Static radiation
    seriesList[staticRadiationPosition]->append(t, p.radsens.staticRadiation);
    adjustAxes(staticRadiationPosition, t, (float)p.radsens.staticRadiation);

    // Dynamic radiation
    seriesList[dynamicRadiationPosition]->append(t, p.radsens.dynamicRadiation);
    adjustAxes(dynamicRadiationPosition, t, (float)p.radsens.dynamicRadiation);

    // Number of impulses (pulses)
    seriesList[impulsesPosition]->append(t, p.radsens.numberOfPulses);
    adjustAxes(impulsesPosition, t, (float)p.radsens.numberOfPulses);

    // Gyroscope (MPU6050 magnitude)
    {
        float magGyro = std::sqrt(
            p.mpu6050Data.gyroX * p.mpu6050Data.gyroX +
            p.mpu6050Data.gyroY * p.mpu6050Data.gyroY +
            p.mpu6050Data.gyroZ * p.mpu6050Data.gyroZ
            );
        seriesList[gyroscopePostition]->append(t, magGyro);
        adjustAxes(gyroscopePostition, t, magGyro);
    }

    // Angle (MPU6050 magnitude)
    {
        float magAng = std::sqrt(
            p.mpu6050Data.angX * p.mpu6050Data.angX +
            p.mpu6050Data.angY * p.mpu6050Data.angY +
            p.mpu6050Data.angZ * p.mpu6050Data.angZ
            );
        seriesList[anglePosition]->append(t, magAng);
        adjustAxes(anglePosition, t, magAng);
    }

    // Acceleration (MPU6050 magnitude)
    {
        float magAcc = std::sqrt(
            p.mpu6050Data.accX * (float)p.mpu6050Data.accX +
            p.mpu6050Data.accY * (float)p.mpu6050Data.accY +
            p.mpu6050Data.accZ * (float)p.mpu6050Data.accZ
            );
        seriesList[accelerationPosition]->append(t, magAcc);
        adjustAxes(accelerationPosition, t, magAcc);
    }

    // Gyroscope 2 (MPU9250 magnitude)
    {
        float magGyro2 = std::sqrt(
            p.mpu9250Data.gyroX * p.mpu9250Data.gyroX +
            p.mpu9250Data.gyroY * p.mpu9250Data.gyroY +
            p.mpu9250Data.gyroZ * p.mpu9250Data.gyroZ
            );
        seriesList[gyroscopePostition2]->append(t, magGyro2);
        adjustAxes(gyroscopePostition2, t, magGyro2);
    }

    // Angle 2 (MPU9250 magnitude)
    {
        float magAng2 = std::sqrt(
            p.mpu9250Data.angX * p.mpu9250Data.angX +
            p.mpu9250Data.angY * p.mpu9250Data.angY +
            p.mpu9250Data.angZ * p.mpu9250Data.angZ
            );
        seriesList[anglePosition2]->append(t, magAng2);
        adjustAxes(anglePosition2, t, magAng2);
    }

    // Acceleration 2 (MPU9250 magnitude)
    {
        float magAcc2 = std::sqrt(
            p.mpu9250Data.accX * (float)p.mpu9250Data.accX +
            p.mpu9250Data.accY * (float)p.mpu9250Data.accY +
            p.mpu9250Data.accZ * (float)p.mpu9250Data.accZ
            );
        seriesList[accelerationPosition2]->append(t, magAcc2);
        adjustAxes(accelerationPosition2, t, magAcc2);
    }

    // active time
    ui->label_motor_status_data->setText(QString::number(t));

    // voltage
    ui->label_voltage_data->setText(QString::number(p.voltage));

    // packet received
    ui->label_packet_sent_data->setText(QString::number(p.numberOfPacket));

    // date
    ui->label_date_data->setText(QString::number(p.gpsData.date.day) + "/" + QString::number(p.gpsData.date.month) + "/" + QString::number(p.gpsData.date.year));

    // time
    ui->label_time_data->setText(QString::number(p.gpsData.time.hour) + ":" + QString::number(p.gpsData.time.minute) + ":" + QString::number(p.gpsData.time.second));

    // motor status
    bool motorStatus = p.backsticks.motorStatus;
    if (motorStatus) {
        ui->label_motor_status_data->setText("ON");
    } else {
        ui->label_motor_status_data->setText("OFF");
    }

    // active time
    ui->label_active_time_data->setText(QString::number(p.timeActive));

    // CanSat mode
    int mode = p.mode;
    if (mode == 1) {
        ui->label_mode_data->setText("Launch Pad Mode");
    } else if (mode == 2) {
        ui->label_mode_data->setText("Ascent Mode");
    } else if (mode == 3) {
        ui->label_mode_data->setText("Can Back Preparation Mode");
    } else if (mode == 4) {
        ui->label_mode_data->setText("Can Back Mode");
    } else if (mode == 5) {
        ui->label_mode_data->setText("Can Landing Mode");
    } else if (mode == 6) {
        ui->label_mode_data->setText("Can Recovery Mode");
    } else {
        qDebug() << "Cansat mode value with a meaninguless value: " << QString::number(p.mode);
    }
}

void MainWindow::adjustAxes(int index, float xValue, float yValue) {
    auto *axisX = xList[index];
    auto *axisY = yList[index];

    if (xValue > axisX->max()) {
        axisX->setMax(xValue + 5);
    }

    if (yValue > axisY->max()) {
        axisY->setMax(yValue + 5);
    } else if (yValue < axisY->min()) {
        axisY->setMin(yValue - 5);
    }
}


void MainWindow::on_actionClear_charts_triggered()
{
    float timeActive = packetList.back().timeActive;

    for (int i = 0; i < xList.size(); i++) {
        xList.at(i)->setRange(timeActive, 100 + timeActive);
    }

    for (int i = 0; i < yList.size(); i++) {
        yList.at(i)->setRange(timeActive, 100 + timeActive);
    }

    for (int i = 0; i < seriesList.size(); i++) {
        seriesList.at(i)->clear();
    }
}

void MainWindow::printPacket () {
    ui->console->insertPlainText(buffer);
    if (autoScrollState) {
        ui->console->moveCursor(QTextCursor::End);
    }
}



void MainWindow::on_pushButton_send_clicked()
{
    QString textToSend = ui->lineEdit_send->text();
    QByteArray bytes = textToSend.toUtf8();

    if (serial->isOpen()) {
        qint64 written = serial->write(bytes);
        if (written == -1) {
            QMessageBox::warning(this, "Send Error",
                                 "Failed to write to serial port: " + serial->errorString());
        }
    } else {
        QMessageBox::warning(this, "Not Connected",
                             "Serial port is not open");
    }
}


MainWindow::~MainWindow() {
    delete serial;
    delete ui;
}

