#include "maroloDAQ.h"
#include "ui_maroloDAQ.h"

#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string>

maroloDAQ::maroloDAQ(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::maroloDAQ)
{
    ui->setupUi(this);
    
    // Procurando por portar seriais abertas
    scanPortas();
    
    // habilitando|desabilitando menus|actions
    ui->btnAppClose->setEnabled(true);
    ui->actionSair->setEnabled(true);
    ui->actionConectar->setEnabled(true);
    ui->actionDesconectar->setEnabled(false);
    ui->actionRecarregar->setEnabled(true);
    
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

void maroloDAQ::findMaroloDAQ() {
    
    QString GetInfoHw;
    QStringList InfoHW;

    /* Create Object the Class QSerialPort*/
    devserial = new QSerialPort(this);
    /* Create Object the Class comserial to manipulate read/write of the my way */
    procSerial = new comserial(devserial);
    /* Load Device PortSerial available */
    QStringList DispSeriais = procSerial->CarregarDispositivos();

    // algumas variaveis temporarias    
    int count = DispSeriais.count();
    //const char * meuAction;
    //const char * meuAction_tmp;
    QString minhaSerial;
    
    if(DispSeriais.length() > 0) {
        
        // algumas variaveis temporarias
        bool ismarolo = false;
        
        for(int i=0;i<count;i++) {
            
            // portas seriais com dispositivos conectados serao enviado ao menu Ferramentas.
            for(int i=0;i<count;i++) {
                //meuAction_tmp = DispSeriais[i].toStdString().c_str();
                //meuAction = meuAction_tmp;

                minhaSerial = DispSeriais[i];
                //ui->menuPortas->addAction("/dev/"+minhaSerial);
                //ui->menuPortas->addAction("/dev/"+minhaSerial,this,SLOT(on_actionConectar_triggered()));
                //connect(meuAction, SIGNAL(triggered()),this,SLOT(findMaroloDAQ()));
                
                //qDebug() << "AQUI meuAction = " << meuAction << endl;
                //qDebug() << "AQUI minhaSerial = " << minhaSerial << endl;                    
            }
            
            if (!ismarolo) {
                
                /*
                 *    statusOpenSerial = procSerial->Conectar(ui->cbDevList->currentText(),
                 *                                            ui->cbDevBaudRate->currentText().toInt()
                 *                                            );
                 */
                
                /* Preciso melhorar isso para pegar o conteúdo dos submenus */
                procSerial->Conectar(DispSeriais[i],9600);
                
                /*
                 *               const char * meuAction = "action";
                 *               const char * meuAction_tmp;
                 *               meuAction_tmp = DispSeriais[i].toStdString().c_str();
                 *               meuAction = meuAction_tmp;
                 */
                
                /*
                 * aguardando a porta "aquecer" ;_(((
                 *
                 */
                sleep(2);
                
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
                    /*
                     *                   for(int i=0;i<count;i++) {
                     *                       ui->menuPortas->addAction("/dev/"+DispSeriais[i]);
                     *                       //ui->menuPortas->addAction("/dev/"+DispSeriais[i],this,SLOT(myaction()));
                }
                */
                    // habilitando|desabilitando menus|actions
                    ui->btnAppClose->setEnabled(false);
                    ui->actionSair->setEnabled(false);
                    ui->actionConectar->setEnabled(false);
                    ui->actionDesconectar->setEnabled(true);
                    ui->actionRecarregar->setEnabled(false);
                    
                    // Inserindo nos devidos Edits
                    ui->editDevCompiler->setText(InfoHW[0]);
                    ui->editDevModel->setText(InfoHW[1]);
                    
                    //ui->menuPortas->addAction("[maroloDAQ] /dev/"+DispSeriais[i]);
                    //setPortasSeriais(DispSeriais[i]+" maroloDAQ");
                    
                    // escrevendo no terminal
                    ui->teLog->append("### maroloDAQ Aberto com Sucesso!");
                    
                    ismarolo = true;
                }  
            }
        }
        if (!ismarolo) {
            // habilitando|desabilitando menus|actions
            ui->btnAppClose->setEnabled(false);
            ui->actionSair->setEnabled(true);
            ui->actionConectar->setEnabled(false);
            ui->actionDesconectar->setEnabled(false);
            ui->actionRecarregar->setEnabled(true);

            ui->teLog->append("### Erro ao obter informações do maroloDAQ, tente novamente.");
        }
    }
    else {
        
        // habilitando|desabilitando menus|actions
        ui->btnAppClose->setEnabled(true);
        ui->actionSair->setEnabled(true);
        ui->actionConectar->setEnabled(false);
        ui->actionDesconectar->setEnabled(false);
        ui->actionRecarregar->setEnabled(true);
        
        ui->teLog->append("### Nenhuma porta serial foi detectada!");
    }
}


void maroloDAQ::scanPortas() {
    
    QString GetInfoHw;
    QStringList InfoHW;
    
    /* Create Object the Class QSerialPort*/
    devserial = new QSerialPort(this);
    /* Create Object the Class comserial to manipulate read/write of the my way */
    procSerial = new comserial(devserial);
    /* Load Device PortSerial available */
    QStringList DispSeriais = procSerial->CarregarDispositivos();
    // fechando tudo
    bool statusCloseSerial;
    
    // algumas variaveis temporarias    
    int count = DispSeriais.count();
    //const char * meuAction;
    //const char * meuAction_tmp;
    QString minhaSerial;
    
    if(DispSeriais.length() > 0) {
        
        for(int i=0;i<count;i++) {
            
            minhaSerial = "/dev/"+DispSeriais[i];
            
            procSerial->Conectar(minhaSerial,9600);
            
            // aguardando a porta "aquecer" ;_(((
            sleep(2);
            
            // Se conectou com sucesso no disposito serial
            
            // Enviando comando para obter informações do Device
            WriteData("12\n");
            // * Recebendo as informações *
            GetInfoHw = ReadData();
            GetInfoHw = GetInfoHw.simplified();
            
            // * Confirmando se recebeu os dados *
            if( GetInfoHw.size() > 0 ) {
                
                // encontrei maroloDAQ
                minhaSerial = minhaSerial+" [maroloDAQ]";
                // escrevendo no terminal
                ui->teLog->append("### maroloDAQ ABERTO com Sucesso!");
                
                // fechando a serial com o maroloDAQ
                statusCloseSerial = procSerial->Desconectar();
                
                if (!statusCloseSerial) {
                    ui->teLog->append("### FALHA AO FECHAR SERIAIS");
                }
                
                // encerrando a busca por maroloDAQ
            }  
            
            // criando action para o submenu
            //minhaSerial = "/dev/"+minhaSerial;
            setPortasSeriais(minhaSerial, i);
            
            //qDebug() << "AQUI minhaSerial = " << minhaSerial << endl;                    
            
        }
    }
    else {
        ui->teLog->append("### Nenhuma porta serial foi detectada!");
    }
}

void maroloDAQ::maroloDevClose()
{
    
    bool statusCloseSerial;
    
    statusCloseSerial = procSerial->Desconectar();
    
    // Descontando a porta serial com sucesso
    // Desabilito os botões Versao, Desconectar, Hardware, Ligar [F10]
    // Habilito Sair e Conectar
    
    if (statusCloseSerial) {
        // habilitando|desabilitando menus|actions
        //ui->btnDevClose->setEnabled(false);
        //ui->btnDevOpen->setEnabled(true);
        ui->btnAppClose->setEnabled(true);
        ui->actionSair->setEnabled(true);
        ui->actionConectar->setEnabled(true);
        ui->actionDesconectar->setEnabled(false);
        ui->actionRecarregar->setEnabled(true);
        
        ui->btnCalibrarSensor->setEnabled(false);
        ui->btnIniciar->setEnabled(false);
        ui->btnParar->setEnabled(false);
        
        ui->editDevCompiler->clear();
        ui->editDevModel->clear();
        
        ui->teLog->append("### Porta serial fechada com sucesso!");
    }
    else {
        ui->teLog->append("### Falha ao fechar conexão serial.");
    }
}

void maroloDAQ::on_btnCalibrarSensor_clicked()
{
    
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
    //findMaroloDAQ();
}

void maroloDAQ::on_btnDevClose_clicked()
{
    maroloDevClose();
}

void maroloDAQ::on_btnParar_clicked()
{
    
}

void maroloDAQ::on_btnIniciar_clicked()
{
    
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
    //findMaroloDAQ();
}

void maroloDAQ::on_actionDesconectar_triggered()
{
    bool statusCloseSerial;
    
    statusCloseSerial = procSerial->Desconectar();
    
    
    // Descontando a porta serial com sucesso
    // Desabilito os botões Versao, Desconectar, Hardware, Ligar [F10]
    // Habilito Sair e Conectar
    
    if (statusCloseSerial) {
        // habilitando|desabilitando menus|actions
        //ui->btnDevClose->setEnabled(false);
        //ui->btnDevOpen->setEnabled(true);
        ui->btnAppClose->setEnabled(true);
        ui->actionSair->setEnabled(true);
        ui->actionConectar->setEnabled(true);
        ui->actionDesconectar->setEnabled(false);
        ui->actionRecarregar->setEnabled(true);
        
        ui->btnCalibrarSensor->setEnabled(false);
        ui->btnIniciar->setEnabled(false);
        ui->btnParar->setEnabled(false);
        
        ui->editDevCompiler->clear();
        ui->editDevModel->clear();
        
        ui->teLog->append("### Porta serial fechada com sucesso!");
    }
    else {
        ui->teLog->append("### Falha ao fechar conexão serial.");
    }
}

void maroloDAQ::on_actionSobre_triggered()
{
    ui->teLog->append("### AQUI on_actionSobre_triggered().");
}

void maroloDAQ::on_actionRecarregar_triggered()
{
    scanPortas();
}

void maroloDAQ::toggleStatusbar(QAction *viewst) {
    
    if (viewst->isChecked()) {
        statusBar()->show();
        
        /*
         *    // debugando o que chega aqui
         *    void enumerateMenu(QMenu *menu)
         *    foreach (viewst, menuBar()) {
         *        if (action->isSeparator()) {
         *            qDebug("this action is a separator");
    } else if (action->menu()) {
        qDebug("action: %s", qUtf8Printable(action->text()));
        qDebug(">>> this action is associated with a submenu, iterating it recursively...");
        enumerateMenu(action->menu());
        qDebug("<<< finished iterating the submenu");
    } else {
        qDebug("action: %s", qUtf8Printable(action->text()));
    }
    }
    */
        
    } else {
        statusBar()->hide();
    }
}

void maroloDAQ::setPortasSeriais(QString myAction, int serial_index) {
    //////////////////////////////////////////////////////
    //ref.: http://zetcode.com/gui/qt5/menusandtoolbars/
    
    QString myAction_spare;
    QString myAction_temp;
    QStringList myAction_split;
    myAction = myAction.simplified();
    myAction_temp = myAction;
    myAction_split = myAction.split(" [");
    if (myAction_split.length() > 1) {
        myAction_temp = myAction_split[0];
        //qDebug() << "# AQUI myAction_split[0] = " << myAction_split[0];
        //qDebug() << "# AQUI myAction_split[1] = " << myAction_split[1];
    }
    
    int ihavename = 0;

    foreach (QAction *action, ui->menuPortas->actions()) {
        if ( !(action->isSeparator()) || !(action->menu()) ) {
            myAction_spare = action->text();
            myAction_spare = myAction_spare.simplified();
            myAction_split = myAction_spare.split(" [");
            if (myAction_split.length() > 1) {
                myAction_spare = myAction_split[0];
                //qDebug() << "# AQUI myAction_split[0] = " << myAction_split[0];
                //qDebug() << "# AQUI myAction_split[1] = " << myAction_split[1];
            }
            qDebug() << "# AQUI myAction_temp = " << myAction_temp;// << endl;
            qDebug() << "# AQUI myAction_spare = " << myAction_spare;

            
            //if ( QString::compare(myAction_temp, myAction_spare, Qt::CaseInsensitive) ) {
            if ( myAction_temp == myAction_spare ) {
                ihavename = ihavename + 1;
                //qDebug() << "# AQUI myAction_temp = " << myAction_temp;// << endl;
                qDebug("# action->text() isn't menu: %s", qUtf8Printable(action->text()));
                qDebug() << "#####################################################";
            }
        }
    }
    //qDebug() << "# AQUI ihavename = " << ihavename;
    if ( ihavename == 0 ) {
        //myAction="/dev/"+myAction;
        qDebug() << "##################################################################";
        qDebug() << "###### CRIANDO SUBMENU -- myAction = " << myAction;
        qDebug() << "##################################################################";
        viewst = new QAction(myAction, this);
        viewst->setData(serial_index);
        viewst->setCheckable(true);
        viewst->setChecked(false);
        
        ui->menuPortas->addAction(viewst);
        
        // debugando
        //enumerateMenu(ui->menuPortas);    
        
        statusBar();
        
        //connect(viewst, &QAction::triggered, this, &maroloDAQ::toggleStatusbar());
        connect(ui->menuPortas, SIGNAL(triggered(QAction*)), this, SLOT(toggleStatusbar(QAction*)));
    }
}

void maroloDAQ::enumerateMenu(QMenu *menu) {
    //////////////////////////////////////////////////////
    //ref.: https://stackoverflow.com/questions/9399840/how-to-iterate-through-a-menus-actions-in-qt
    
    foreach (QAction *action, menu->actions()) {
        
        if (action->isSeparator()) {
            qDebug() << "this action is a separator.";// << "" << endl;
        } else if (action->menu()) {
            qDebug() << "action1: %s:" << qUtf8Printable(action->text());// << endl;
            qDebug() << "this action is associated with a submenu, iterating it recursively...";// << "" << endl;
            enumerateMenu(action->menu());
            qDebug() << "finished iterating the submenu";// << "" << endl;
        } else {
            qDebug() << "action2: %s" << qUtf8Printable(action->text());// << endl;
        }
    }
}
