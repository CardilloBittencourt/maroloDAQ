#include "maroloDAQ.h"
#include "ui_maroloDAQ.h"
#include "calibration.h"

#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string>
#include <math.h>

maroloDAQ::maroloDAQ(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::maroloDAQ)
{
    ui->setupUi(this);
    
    // Formatando entradas
    ui->editAngulo1->setValidator(new QIntValidator(-90,90,ui->editAngulo1));
    ui->editAngulo1->setMaxLength(3);
    //ui->editAngulo1->setInputMask("#99");

    ui->editAngulo2->setValidator(new QIntValidator(-90,90,ui->editAngulo2));
    ui->editAngulo2->setMaxLength(3);
    //ui->editAngulo2->setInputMask("#99");
    
    QDoubleValidator *dvVal1 = new QDoubleValidator(0,1e99,1,ui->editErroSensor);
    dvVal1->setNotation(QDoubleValidator::ScientificNotation);
    ui->editErroSensor->setValidator(dvVal1);
    ui->editErroSensor->setMaxLength(7);
    //ui->editErroSensor->setInputMask("9e#99");
    
    ui->editDeltaT->setValidator(new QDoubleValidator(0,99999,1,ui->editDeltaT));
    //dvVal2->setNotation(QDoubleValidator::ScientificNotation);
    //ui->editDeltaT->setValidator(dvVal2);
    ui->editDeltaT->setMaxLength(5);
    //ui->editDeltaT->setInputMask("99999");

    ui->editTmax->setValidator(new QDoubleValidator(0,999999,1,ui->editErroSensor));
    //dvVal3->setNotation(QDoubleValidator::ScientificNotation);
    //ui->editTmax->setValidator(dvVal3);
    ui->editTmax->setMaxLength(6);
    //ui->editTmax->setInputMask("999999");
    
    ui->editDevCompiler->setReadOnly(true);
    ui->editDevModel->setReadOnly(true);

    // Procurando por portar seriais abertas
    scanPortas();
    
    setDesconectado();

    //Conectando SIGNAL de Timer a função update
    //connect(timer, SIGNAL(timeout()), this, SLOT(update()));
}

maroloDAQ::~maroloDAQ()
{
    delete ui;
}

void maroloDAQ::on_btnAppClose_clicked()
{
    exit(0);
}

void maroloDAQ::WriteData(const QByteArray data)
{
    procSerial->Write(data);
}

QString maroloDAQ::ReadData()
{
    QString data = procSerial->Read();
    //qDebug() << "ReadData - RX UART: " << data << endl;
    return data;
}

void maroloDAQ::scanPortas() {
    
    QString GetInfoHw;
    QStringList InfoHW;

    // Serials Port Group
    PortasGroup = new QActionGroup(this);
    QStringList myAction_split;
    
    /* Create Object the Class QSerialPort*/
    devserial = new QSerialPort(this);
    /* Create Object the Class comserial to manipulate read/write of the my way */
    procSerial = new comserial(devserial);
    /* Load Device PortSerial available */
    QStringList DispSeriais = procSerial->CarregarDispositivos();
    
    bool statusCloseSerial;
    bool statusOpenSerial;
    
    // algumas variaveis temporarias    
    int count = DispSeriais.count();
    //const char * meuAction;
    //const char * meuAction_tmp;
    QString minhaSerial;
    
    if(count > 0) {
        
        foreach (QAction* action, ui->menuPortas->actions()) {
            PortasGroup->removeAction(action);
            ui->menuPortas->removeAction(action);
        }
        
        for(int i=0;i<count;i++) {
            
            minhaSerial = DispSeriais[i];
            
            //qDebug() << "AQUI minhaSerial = " << minhaSerial << endl;
            
            statusOpenSerial = procSerial->Conectar(minhaSerial,9600);
            
            // aguardando a porta "aquecer" ;_(((
            sleep(2);
            
            if (statusOpenSerial) {
                
                // Se conectou com sucesso no disposito serial
                
                // Enviando comando para obter informações do Device
                WriteData("12\n");
                // * Recebendo as informações *
                GetInfoHw = ReadData();
                GetInfoHw = GetInfoHw.simplified();
                
                //qDebug() << "----------> AQUI GetInfoHW = " << GetInfoHw << endl;
                
                // * Confirmando se recebeu os dados *
                if( GetInfoHw.size() > 0 ) {
                    
                    // encontrei maroloDAQ
                    minhaSerial = minhaSerial+" [maroloDAQ]";
                    //setPortasSeriais(minhaSerial+" [maroloDAQ]");
                    
                    //qDebug() << "AQUI minhaSerial = " << minhaSerial+"[ maroloDAQ]" << endl;
                    
                    // escrevendo no terminal
                    //ui->teLog->append("### maroloDAQ ABERTO com Sucesso!");
                    
                }
                
                setPortasSeriais(minhaSerial);
                
                statusCloseSerial = procSerial->Desconectar();
                
                if (statusCloseSerial) {
                }
                else {
                    //qDebug() << "FALHA ao FECHAR Serial dev = " << minhaSerial;
                }
                
                //qDebug() << "AQUI minhaSerial = " << minhaSerial << endl;
            }
            else {
                //qDebug() << "FALHA ao ABRIR Serial dev = " << minhaSerial; 
            }
        }
        
        // ref.: https://stackoverflow.com/questions/9399840/how-to-iterate-through-a-menus-actions-in-qt
        foreach (QAction* action, ui->menuPortas->actions()) {
            PortasGroup->addAction(action);
            
            myAction_split = action->text().split("]");
            //qDebug() << "$$$$ AQUI action = " << action->text();
            if ( myAction_split.length() > 1 ) {
                if (  myAction_split[1] == "maroloDAQ]") {
                    action->setCheckable(true);
                    action->setChecked(true);
                }
            }
            else {
                action->setCheckable(true);
                action->setChecked(false);
            }
        }   
    }
    else {
        ui->teLog->append("### Nenhuma porta serial foi detectada!");
    }
    /*
    // ref.: https://stackoverflow.com/questions/9399840/how-to-iterate-through-a-menus-actions-in-qt
    foreach (QAction* action, ui->menuPortas->actions()) {
        PortasGroup->addAction(action);
        
        myAction_split = action->text().split("]");
        //qDebug() << "$$$$ AQUI action = " << action->text();
        if ( myAction_split.length() > 1 ) {
            if (  myAction_split[1] == "maroloDAQ]") {
                action->setCheckable(true);
                action->setChecked(true);
            }
        }
        else {
            action->setCheckable(true);
            action->setChecked(false);
        }
    }
    */
}

void maroloDAQ::setDesconectado() {
    
    // limpando os edits
    ui->editDevCompiler->clear();
    ui->editDevModel->clear();
    ui->editAngulo1->clear();
    ui->editAngulo2->clear();
    ui->editErroSensor->clear();
    ui->editDeltaT->clear();    
    ui->editTmax->clear();

    // habilitando|desabilitando menus|actions
    ui->btnAppClose->setEnabled(true);
    ui->btnDevOpen->setEnabled(true);
    ui->btnDevClose->setEnabled(false);
    ui->gbSensor->setEnabled(true);
    ui->gbTempo->setEnabled(true);
    ui->gbCalibrar->setEnabled(true);
    ui->gbMonitor->setEnabled(true);
    ui->btnCalibrar1->setEnabled(false);
    ui->btnCalibrar2->setEnabled(false);
    ui->cbSensorList->setEnabled(false);
    ui->cbPinoList->setEnabled(false);
    ui->btnIniciar->setEnabled(false);
    ui->btnParar->setEnabled(false);
    ui->actionSair->setEnabled(true);
    ui->actionConectar->setEnabled(true);
    ui->actionDesconectar->setEnabled(false);
    ui->actionRecarregar->setEnabled(true);
    
    ui->editDevCompiler->setEnabled(false);
    ui->editDevModel->setEnabled(false);
    ui->editAngulo1->setEnabled(false);
    ui->editAngulo2->setEnabled(false);
    ui->editErroSensor->setEnabled(false);
    ui->editDeltaT->setEnabled(false);    
    ui->editTmax->setEnabled(false);
    }

void maroloDAQ::setConectado() {

    // limpando os edits
    //ui->editDevCompiler->clear();
    //ui->editDevModel->clear();
    ui->editAngulo1->clear();
    ui->editAngulo2->clear();
    ui->editErroSensor->clear();
    ui->editDeltaT->clear();    
    ui->editTmax->clear();
    
    // habilitando|desabilitando menus|actions
    ui->btnAppClose->setEnabled(false);
    ui->btnDevOpen->setEnabled(false);
    ui->btnDevClose->setEnabled(true);
    ui->gbSensor->setEnabled(true);
    ui->gbTempo->setEnabled(true);
    ui->gbCalibrar->setEnabled(true);
    ui->gbMonitor->setEnabled(true);
    ui->btnCalibrar1->setEnabled(false);
    ui->btnCalibrar2->setEnabled(false);
    ui->cbSensorList->setEnabled(true);
    ui->cbPinoList->setEnabled(true);
    //modificado para teste btnIniciar
    ui->btnIniciar->setEnabled(true);
    ui->btnParar->setEnabled(false);
    ui->actionSair->setEnabled(false);
    ui->actionConectar->setEnabled(false);
    ui->actionDesconectar->setEnabled(true);
    ui->actionRecarregar->setEnabled(false);

    ui->editDevCompiler->setEnabled(true);
    ui->editDevModel->setEnabled(true);
    ui->editAngulo1->setEnabled(false);
    ui->editAngulo2->setEnabled(false);
    ui->editErroSensor->setEnabled(true);
    ui->editDeltaT->setEnabled(true);    
    ui->editTmax->setEnabled(true);
}

void maroloDAQ::maroloDevClose()
{
    bool statusCloseSerial;

    // Serials Port Group
    PortasGroup = new QActionGroup(this);
    // removing all ations in PortasGroup
    QStringList myAction_split;
    
    foreach (QAction* action, ui->menuPortas->actions()) {
        PortasGroup->removeAction(action);
        ui->menuPortas->removeAction(action);
    }
    
    statusCloseSerial = procSerial->Desconectar();
    
    // Descontando a porta serial com sucesso
    // Desabilito os botões Versao, Desconectar, Hardware, Ligar [F10]
    // Habilito Sair e Conectar
    
    if (statusCloseSerial) {
        
        ui->editDevCompiler->clear();
        ui->editDevModel->clear();
        
        setDesconectado();

        ui->teLog->append("### Porta serial fechada com sucesso!");
    }
    else {
        ui->teLog->append("### Falha ao fechar conexão serial.");
    }
}

void maroloDAQ::on_btnBWTerminal_clicked()
{
    QPalette paleta;
    /*
     * Verifica se PaletaBW é True ou False
     * Se True: Fundo Preto, Fonte Branco
     * Se False: Fundo Branco, Fonte Preto
     */
    
    if(PaletaLogBW) {
        paleta.setColor(QPalette::Base,Qt::black);
        paleta.setColor(QPalette::Text,Qt::white);
        ui->teLog->setPalette(paleta);
        PaletaLogBW=false;
    }
    else {
        paleta.setColor(QPalette::Base,Qt::white);
        paleta.setColor(QPalette::Text,Qt::black);
        ui->teLog->setPalette(paleta);
        PaletaLogBW=true;
    }
}


void maroloDAQ::on_btnDevOpen_clicked()
{
    QString GetInfoHw;
    QStringList InfoHW;
    QString devport;
    QStringList devport_list;
    bool statusOpenSerial;
    
    foreach (QAction* action, ui->menuPortas->actions()) {
        if (action->isChecked()) {
            devport = action->text();
            devport = devport.simplified();
            devport_list = devport.split(" [");
            devport = devport_list[0];
        }    
    }
    
    //qDebug() << "AQUI btnDevOpen: devport = " << devport;
    statusOpenSerial = procSerial->Conectar(devport,9600);
    
    /*
     * aguardando a porta "aquecer" ;_(((
     *
     */
    sleep(2);
    
    if (statusOpenSerial) {
        /*
         * Se conectou com sucesso no disposito serial
         * Desabilito o botão Conectar e Sair
         * Habilito Desconectar, Versao, Hardware e Ligar [F10]
         */
        
        // * Enviando comando para obter informações do Device *
        WriteData("12\n");
        // * Recebendo as informações *
        GetInfoHw = ReadData();
        GetInfoHw = GetInfoHw.simplified();
        
        // * Confirmando se recebeu os dados *
        if( GetInfoHw.size() > 0 ) {
            
            
            // * Ex: 4.3.2|UNO
            //  * O que chegou pela serial foi adicionado na variavel GetInfoHW
            //  * então acima removemos qualquer tabulação e abaixo um split
            //  * baseado no caractere |, então sera quebrado em 2 posicoes
            //  * 0 - 4.3.2
            //  * 1 - UNO
            //  *
            InfoHW = GetInfoHw.split("|");
            
            // Inserindo nos devidos Edits
            ui->editDevCompiler->setText(InfoHW[0]);
            ui->editDevModel->setText(InfoHW[1]);
            
            setConectado();
            
            ui->teLog->append("### maroloDAQ Aberto com Sucesso!");
        }
        else {
            ui->teLog->append("### Erro ao obter informações do maroloDAQ, tente novamente.");
        }
    }
    else {
        ui->teLog->append("### FALHA ao ABRIR Porta Serial. Tente de Novo!");
    }
    
} // end on_btnDevOpen_clicked

void maroloDAQ::on_btnDevClose_clicked()
{
    bool statusCloseSerial;
    
    statusCloseSerial = procSerial->Desconectar();
    
    if (statusCloseSerial) {
        setDesconectado();        
        ui->teLog->append("### Porta serial fechada com sucesso!");
    }
    else {
        ui->teLog->append("### Falha ao fechar conexão serial.");
    }
}

void maroloDAQ::on_btnParar_clicked()
{
    ui->editErroSensor->setEnabled(true);
    ui->editDeltaT->setEnabled(true);
    ui->editTmax->setEnabled(true);
    ui->btnIniciar->setEnabled(true);
    ui->btnParar->setEnabled(false);
    ui->cbPinoList->setEnabled(true);
    ui->cbSensorList->setEnabled(true);

    //nao testado
    /*
    if(timer->isActive()){
        timer->stop();
        amostras = 0;
    }
    */

}

void maroloDAQ::on_btnIniciar_clicked()
{
    if(validarEntradas())
    {
        //hablita ou desabilita entradas
        ui->editErroSensor->setEnabled(false);
        ui->editDeltaT->setEnabled(false);
        ui->editTmax->setEnabled(false);
        ui->btnIniciar->setEnabled(false);
        ui->btnParar->setEnabled(true);
        ui->cbPinoList->setEnabled(false);
        ui->cbSensorList->setEnabled(false);

        // inicia medicoes
        doReadings();
    }
}

void maroloDAQ::on_actionSalvar_como_triggered()
{
    
}

void maroloDAQ::on_actionSalvar_triggered()
{
    
}

void maroloDAQ::on_actionSair_triggered()
{
    exit(0);
}

void maroloDAQ::on_actionConectar_triggered()
{
    QString GetInfoHw;
    QStringList InfoHW;
    QString devport;
    QStringList devport_list;
    bool statusOpenSerial;
    
    foreach (QAction* action, ui->menuPortas->actions()) {
        if (action->isChecked()) {
            devport = action->text();
            devport = devport.simplified();
            devport_list = devport.split(" [");
            devport = devport_list[0];
        }    
    }
    
    //qDebug() << "AQUI btnDevOpen: devport = " << devport;
    statusOpenSerial = procSerial->Conectar(devport,9600);
    
    /*
     * aguardando a porta "aquecer" ;_(((
     *
     */
    sleep(2);
    
    if (statusOpenSerial) {
        /*
         * Se conectou com sucesso no disposito serial
         * Desabilito o botão Conectar e Sair
         * Habilito Desconectar, Versao, Hardware e Ligar [F10]
         */
        
        // * Enviando comando para obter informações do Device *
        WriteData("12\n");
        // * Recebendo as informações *
        GetInfoHw = ReadData();
        GetInfoHw = GetInfoHw.simplified();
        
        // * Confirmando se recebeu os dados *
        if( GetInfoHw.size() > 0 ) {
            
            
            // * Ex: 4.3.2|UNO
            //  * O que chegou pela serial foi adicionado na variavel GetInfoHW
            //  * então acima removemos qualquer tabulação e abaixo um split
            //  * baseado no caractere |, então sera quebrado em 2 posicoes
            //  * 0 - 4.3.2
            //  * 1 - UNO
            //  *
            InfoHW = GetInfoHw.split("|");
            
            // Inserindo nos devidos Edits
            ui->editDevCompiler->setText(InfoHW[0]);
            ui->editDevModel->setText(InfoHW[1]);
            
            setConectado();
            
            ui->teLog->append("### maroloDAQ Aberto com Sucesso!");
        }
        else {
            ui->teLog->append("### Erro ao obter informações do maroloDAQ, tente novamente.");
        }
    }
    else {
        ui->teLog->append("### FALHA ao ABRIR Porta Serial. Tente de Novo!");
    }
    
} // end on_actionConectar_triggered

void maroloDAQ::on_actionDesconectar_triggered()
{
    bool statusCloseSerial;
    
    
    statusCloseSerial = procSerial->Desconectar();
    
    if (statusCloseSerial) {
        setDesconectado();
        
        ui->teLog->append("### Porta serial fechada com sucesso!");
    }
    else {
        ui->teLog->append("### Falha ao fechar conexão serial.");
    }
}

void maroloDAQ::on_actionRecarregar_triggered()
{
    /*
    // Serials Port Group
    PortasGroup = new QActionGroup(this);
    // removing all ations in PortasGroup
    QStringList myAction_split;
    
    foreach (QAction* action, ui->menuPortas->actions()) {
        PortasGroup->removeAction(action);
        ui->menuPortas->removeAction(action);
    }
    */
    scanPortas();
}

void maroloDAQ::setPortasSeriais(QString myAction) {
    
    //qDebug() << ">>>>> AQUI myAction = " << myAction;
    
    //////////////////////////////////////////////////////
    //ref.: http://zetcode.com/gui/qt5/menusandtoolbars/
    QString myAction_spare;
    QString myAction_temp;
    QStringList myAction_split;
    myAction = myAction.simplified();
    //myAction_temp = myAction;
    myAction_split = myAction.split(" [");
    myAction_temp = myAction_split[0];
    myAction = "/dev/"+myAction;
    //qDebug() << ">>>>> AQUI myAction = " << myAction;
    
    int ihavename = 0;
    foreach (QAction *action, ui->menuPortas->actions()) {
        if ( !(action->isSeparator()) || !(action->menu()) ) {
            //if ( QString::compare(myAction_temp, myAction_spare, Qt::CaseInsensitive) ) {
            if ( myAction == action->text() ) {
                ihavename = ihavename + 1;
                //qDebug() << "# AQUI myAction = " << myAction;
                //qDebug() << "# AQUI action->text() = " << action->text();
                //qDebug("# action->text() isn't menu: %s", qUtf8Printable(action->text()));
                //qDebug() << "#####################################################";
            }
        }
    }
    if ( ihavename == 0 ) {
        //myAction="/dev/"+myAction;
        //qDebug() << "##################################################################"
        //qDebug() << "###### CRIANDO SUBMENU -- myAction = " << myAction;
        //qDebug() << "##################################################################";
        
        //qDebug() << ">>>>> AQUI myAction_temp = " << myAction_temp;
        
        if ( myAction_temp == "ttyACM0" ) {
            //qDebug() << ">>>>> AQUI myAction_split[1] = " << myAction_split[1];
            actionACM0 = new QAction(myAction, this);
            ui->menuPortas->addAction(actionACM0);
            if (myAction_split.length() > 1 ) {
                if ( myAction_split[1] == "maroloDAQ]") {
                    actionACM0->setCheckable(true);
                    actionACM0->setChecked(true);
                }
            }
        }
        if ( myAction_temp == "ttyACM1" ) {
            //qDebug() << ">>>>> AQUI myAction_split[0] = " << myAction_split[0];
            actionACM0 = new QAction(myAction, this);
            ui->menuPortas->addAction(actionACM1);
            if (myAction_split.length() > 1 ) {
                if ( myAction_split[1] == "maroloDAQ]") {
                    actionACM0->setCheckable(true);
                    actionACM0->setChecked(true);
                }
            }
        }
        if ( myAction_temp == "ttyACM2" ) {
            //qDebug() << ">>>>> AQUI myAction_split[0] = " << myAction_split[0];
            actionACM0 = new QAction(myAction, this);
            ui->menuPortas->addAction(actionACM2);
            if (myAction_split.length() > 1 ) {
                if ( myAction_split[1] == "maroloDAQ]") {
                    actionACM0->setCheckable(true);
                    actionACM0->setChecked(true);
                }
            }
        }
        if ( myAction_temp == "ttyACM3" ) {
            //qDebug() << ">>>>> AQUI myAction_split[0] = " << myAction_split[0];
            actionACM0 = new QAction(myAction, this);
            ui->menuPortas->addAction(actionACM3);
            if (myAction_split.length() > 1 ) {
                if ( myAction_split[1] == "maroloDAQ]") {
                    actionACM0->setCheckable(true);
                    actionACM0->setChecked(true);
                }
            }
        }
        if (  myAction_temp == "ttyS0") {
            //qDebug() << ">>>>> AQUI myAction_split[0] = " << myAction_split[0];
            actionS0 = new QAction(myAction, this);
            ui->menuPortas->addAction(actionS0);
            actionS0->setCheckable(true);
        }
        if (  myAction_temp == "ttyS1") {
            //qDebug() << ">>>>> AQUI myAction_split[0] = " << myAction_split[0];
            actionS1 = new QAction(myAction, this);
            ui->menuPortas->addAction(actionS1);
            actionS1->setCheckable(true);
        }
        if (  myAction_temp == "ttyS2") {
            //qDebug() << ">>>>> AQUI myAction_split[0] = " << myAction_split[0];
            actionS2 = new QAction(myAction, this);
            ui->menuPortas->addAction(actionS2);
            actionS2->setCheckable(true);
        }
        if (  myAction_temp == "ttyS3") {
            //qDebug() << ">>>>> AQUI myAction_split[0] = " << myAction_split[0];
            actionS3 = new QAction(myAction, this);
            ui->menuPortas->addAction(actionS3);
            actionS3->setCheckable(true);
        }
    }
    /*
    // ref.: https://stackoverflow.com/questions/9399840/how-to-iterate-through-a-menus-actions-in-qt
    PortasGroup = new QActionGroup(this);
    foreach (QAction* action, ui->menuPortas->actions())
    {
        PortasGroup->addAction(action);
        actions->setCheckable(true);
    }
    */
} // end setPortasSeriais

void maroloDAQ::enumerateMenu(QMenu *menu) {
    //////////////////////////////////////////////////////
    //ref.: https://stackoverflow.com/questions/9399840/how-to-iterate-through-a-menus-actions-in-qt

    foreach (QAction *action, menu->actions()) {
        
        if (action->isSeparator()) {
            //qDebug() << "this action is a separator.";// << "" << endl;
        } else if (action->menu()) {
            //qDebug() << "action1: %s:" << qUtf8Printable(action->text());// << endl;
            //qDebug() << "this action is associated with a submenu, iterating it recursively...";// << "" << endl;
            enumerateMenu(action->menu());
            //qDebug() << "finished iterating the submenu";// << "" << endl;
        } else {
            //qDebug() << "action2: %s" << qUtf8Printable(action->text());// << endl;
        }
    }
}

void maroloDAQ::on_cbSensorList_activated(const QString &arg1) {
    //qDebug() << "----> AQUI cbSensorList e &arg1 = " << arg1;
    if ( arg1 == "Pêndulo" ) {
        ui->editAngulo1->setEnabled(true);
        ui->editAngulo2->setEnabled(true);
    }
    else {
        ui->editAngulo1->setEnabled(false);
        ui->editAngulo2->setEnabled(false);
    }
}

bool maroloDAQ::validarEntradas() {
    //Se ERRO for vazio apresenta mensagem de erro e para operação, senão...
    //Se DeltaT for vazio apresenta mensagem de erro e para operação, senão...
    //Se Tmax for vazio apresenta mensagem de erro e para operação, senão...
    //Tudo ok e continua a operação.
    if(ui->editErroSensor->text()==NULL){
        msgBox.setText("Digite o Erro");
        msgBox.exec();
        ui->editErroSensor->setFocus();
        return false;
    }else{
        if(ui->editDeltaT->text()==NULL){
            msgBox.setText("Digite o intervalo de amostragem");
            msgBox.exec();
            ui->editDeltaT->setFocus();
            return false;
        }else{
            if(ui->editTmax->text()==NULL){
                msgBox.setText("Digite o tempo máximo da amostra");
                msgBox.exec();
                ui->editTmax->setFocus();
                return false;
            }else{
                return true;
            }
        }
    }
} // end validarEntradas

//
void maroloDAQ::doReadings() {
    //Configura myCALL com o valor do pino do Aduino
    QByteArray myCALL;
    switch(ui->cbPinoList->currentIndex())
    {
        case 0:
            // Ler pino A0 no Arduino
            myCALL = "14\n";
            break;
        case 1:
            // Ler pino A1 no Arduino
            myCALL = "15\n";
            break;
        case 2:
            // Ler pino A2 no Arduino
            myCALL = "16\n";
            break;
        case 3:
            // Ler pino A3 no Arduino
            myCALL = "17\n";
            break;
        case 4:
            // Ler pino A4 no Arduino
            myCALL = "18\n";
            break;
        case 5:
            // Ler pino A5 no Arduino
            myCALL = "19\n";
            break;
    }
    
    double mysound,myvoltage,myresistence,mytemperature, mylight,myangle;
    // contador
    int cont = 0;
    //intervalo de tempo para as leituras
    double deltaT = ui->editDeltaT->text().toDouble() * 1000;
    //tempo para as leituras
    double Tmax = ui->editTmax->text().toDouble() * 1000;
    //erro indicado no gui para o sensor
    double erroY = ui->editErroSensor->text().toDouble();
    // tolerância no tempo máximo de leitura
    double tolerance = deltaT * 0.3;

    // definindo o relogio
    //QElapsedTimer timer;
    // inicializando o relogio
    timer.start();
    // instante inicial das medicoes
    double tempo_inicial = timer.elapsed();
    // momento da medicao
    double tempo_atual = 0 ;
    // define timeout
    double timeout = Tmax + tolerance;

    while (!timer.hasExpired(timeout))
    {
        
        if (timer.hasExpired(cont * deltaT))
        {
            
            //Qual Sensor foi Selecionado
            switch(ui->cbSensorList->currentIndex())
            {
                case 0:
                    //mysound = readSOUNLEVEL(myCALL);
                    mysound = 0;
                    // Envia o valor medido ao lcdMonitorY
                    ui->lcdMonitorY->display(QString::number(mysound, 'f', 2));
                    // Envia o tempo decorrido para o lcdMonitorX
                    ui->lcdMonitorX->display(QString::number(tempo_atual, 'f', 2));
                    break;
                case 1:
                    //myvoltage = readVOLTAGE(myCALL);
                    myvoltage = 0;
                    // Envia o valor medido ao lcdMonitorY
                    ui->lcdMonitorY->display(QString::number(myvoltage, 'f', 2));
                    // Envia o tempo decorrido para o lcdMonitorX
                    ui->lcdMonitorX->display(QString::number(tempo_atual, 'f', 2));
                    break;
                case 2:
                    //myresistence = readRESISTENCE(myCALL);
                    myresistence = 0;
                    // Envia o valor medido ao lcdMonitorY
                    ui->lcdMonitorY->display(QString::number(myresistence, 'f', 2));
                    // Envia o tempo decorrido para o lcdMonitorX
                    ui->lcdMonitorX->display(QString::number(tempo_atual, 'f', 2));
                    break;
                case 3:
                    mytemperature = readTemperature(myCALL);
                    // desongelando o GUI
                    QCoreApplication::processEvents();
                    // Envia o valor medido ao lcdMonitorY
                    ui->lcdMonitorY->display(QString::number(mytemperature/10, 'f', 1));
                    // Envia o tempo decorrido para o lcdMonitorX
                    ui->lcdMonitorX->display(QString::number(tempo_atual/1000, 'f', 2));
                    ui->teLog->append((QString::number(tempo_atual/1000, 'f', 2))+"    "+\
				    (QString::number(mytemperature/10, 'f', 1))+"    "+\
				    (QString::number(0.01, 'f', 2))+"    "+\
				    (QString::number(erroY, 'f', 1)));
                    break;
                case 4:
                    //mylight = readLIGHT(myCALL);
                    mylight = 0;
                    // Envia o valor medido ao lcdMonitorY
                    ui->lcdMonitorY->display(QString::number(mylight, 'f', 2));
                    break;
                case 5:
                    // myangle = readPENDULO(myCALL);
                    myangle = 0;
                    // Envia o valor medido ao lcdMonitorY
                    ui->lcdMonitorY->display(QString::number(myangle, 'f', 2));
                    // Envia o tempo decorrido para o lcdMonitorX
                    ui->lcdMonitorX->display(QString::number(tempo_atual, 'f', 2));
                    break;
                    
            } // end switch sensor
            
            cont++;

        } // end if deltaT
	    
	// Atualiza o tempo decorrido na medicao
	tempo_atual = (timer.elapsed() - tempo_inicial);
        
    } // end while timeout

        
    //GUI é reabilitado
    ui->editErroSensor->setEnabled(true);
    ui->editDeltaT->setEnabled(true);
    ui->editTmax->setEnabled(true);
    ui->btnIniciar->setEnabled(true);
    ui->btnParar->setEnabled(false);
    ui->cbPinoList->setEnabled(true);
    ui->cbSensorList->setEnabled(true);

}

double maroloDAQ::readTemperature(QByteArray myCALL) {
    //Envia comando para Arduino ler pino
    WriteData(myCALL);

    //recebe valor lido pelo ADC no pino do sensor
    QString AdcReadString = ReadData();

    //converte String em Inteiro
    double AdcReadDouble = AdcReadString.toDouble();

    //Converte Inteiro em Temperatura
    int temperature = scale_temp(int(AdcReadDouble * (4096/1024)));
    return temperature;

}

//Converte valor de leitura do ADC em valor de temperatura
int maroloDAQ::scale_temp(int adcCount) {
    int i, diffScaled, diffRaw, diffAdc, scaledValue=0;
        double scaleFactor;
        for (i=0; i<100; i++)
        {
            if (adcCount >= temp[i][0] && adcCount < temp[i+1][0])
            {
                    diffScaled = temp[i][1] - temp[i+1][1];
                    diffRaw = temp[i+1][0] - temp[i][0];
                    scaleFactor = ((double)diffScaled ) / (double)diffRaw;
                    diffAdc = adcCount - temp[i][0];
                    scaledValue = temp[i][1] - (diffAdc * scaleFactor);
                    return scaledValue;
            }
    }
    return -1;
}

double maroloDAQ::readAngle(QByteArray myCALL) {
	
	int voltage;
	float angle;
	
	float a;
	float b;
	float pi=3.1415926;
	float v0;
	float v1;
	float teta0;
	float teta1;

	v0 = (float)calibrationArray[0].voltage;	
	v1 = (float)calibrationArray[1].voltage;
	teta0 = (float)(calibrationArray[0].angle);
	teta1 = (float)(calibrationArray[1].angle);
	
	teta0 = (teta0)*(pi/180); // conversion to rad
	teta1 = (teta1)*(pi/180); // conversion to rad
	
	a = (v0-v1)/(sin(teta0)-sin(teta1));
	b = v0-a*sin(teta0);

	voltage = readVoltage(myCALL);

	// Here, conversion voltage into degree
	angle = (180/pi)*asin((voltage-b)/a);
	
	return angle;
}

double maroloDAQ::readVoltage(QByteArray myCALL) {
    //Envia comando para Arduino ler pino
    WriteData(myCALL);

    //recebe valor lido pelo ADC no pino do sensor
    QString AdcReadString = ReadData();

    //converte String em Inteiro
    double AdcReadDouble = AdcReadString.toDouble();

    //Converte Inteiro em Temperatura
    int voltage = (int(AdcReadDouble * (5/1024)));
    return voltage;
}

double maroloDAQ::readLight(QByteArray myCALL) {
    //Envia comando para Arduino ler pino
    WriteData(myCALL);

    //recebe valor lido pelo ADC no pino do sensor
    QString AdcReadString = ReadData();

    //converte String em Inteiro
    double AdcReadDouble = AdcReadString.toDouble();

    //Converte Inteiro em Temperatura
    int light = scale_light(int(AdcReadDouble * (4096/1024)));
    return light;

}

int scale_light(int adcCount)
{
        int i, diffScaled, diffRaw, diffAdc, scaledValue=0;
        double scaleFactor;
        for (i=0; i<10; i++)
        {
                if (adcCount >= light[i][0] && adcCount < light[i+1][0])
                {
                        diffScaled = light[i+1][1] - light[i][1];
                        diffRaw = light[i+1][0] - light[i][0];
                        scaleFactor = (double)diffScaled / (double)diffRaw;
                        diffAdc = adcCount - light[i][0];
                        scaledValue = (diffAdc * scaleFactor) + light[i][1];
                        return scaledValue;
                }
        }
        return -1;
}

double maroloDAQ::readSound(QByteArray myCALL) {
    //Envia comando para Arduino ler pino
    WriteData(myCALL);

    //recebe valor lido pelo ADC no pino do sensor
    QString AdcReadString = ReadData();

    //converte String em Inteiro
    double AdcReadDouble = AdcReadString.toDouble();

    //Converte Inteiro em Temperatura
    int sound = scale_sound(int(AdcReadDouble * (4096/1024)));
    return sound;

}

int scale_sound(int adcCount)
{
        int i, diffScaled, diffRaw, diffAdc, scaledValue=0;
        double scaleFactor;
        for (i=0; i<47; i++)
        {
                if (adcCount >= sound[i][0] && adcCount < sound[i+1][0])
                {
                        diffScaled = sound[i+1][1] - sound[i][1];
                        diffRaw = sound[i+1][0] - sound[i][0];
                        scaleFactor = (double)diffScaled / (double)diffRaw;
                        diffAdc = adcCount - sound[i][0];
                        scaledValue = (diffAdc * scaleFactor) + sound[i][1];
                        return scaledValue;
                }
        }
        return -1;
}
