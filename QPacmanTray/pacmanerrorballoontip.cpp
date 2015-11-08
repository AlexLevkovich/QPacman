/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanerrorballoontip.h"
#include "systemtrayicon.h"

const char * error_str = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                         "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
                         "p, li { white-space: pre-wrap; }"
                         "</style></head><body style=\" font-family:'Segoe UI'; font-size:9pt; font-weight:400; font-style:normal;\">"
                         "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%1</p>"
                         "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                         "<a href=\"button://show\"><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWX"
                         "MAAA3XAAAN1wFCKJt4AAAAB3RJTUUH3wUSCywNVpnszAAAA85JREFUSMe1lV9oHFUUxn/n3pmd7CY1/5dUtCmpaZXUpuhDG1ILBa1KwQrSQsWKRnxSUBBUrNbgi/ZBSsAnaURoBSl"
                         "FKPEhilCRommFINJoQpKatBWrhmzabvbfzM71YWdnZ7epDYIDh3tn7ne+755z55wL//Mjq0IdONWDkc2I6sSIC2oBy5znxN6p/y6w72QcZb2MqAHLUveua03QVG+DCEsZj0uLOTzf"
                         "/IbRQ4gc4/ijy6sXePr0TnyG17Un7tm9Jcn9d9+BYys8H8BgaUXeM/x8Oc3XE4tcXszPgn6GT3eN3V7gwMiTVkx9/tiWpNPf3YIIGCCddXm8N4lSMPrT38QTMQTBAGdnrjN6IeUWj"
                         "ezj2M7TtxZ4dqTP0tZ3Tz241upKJoJVYSld4P39G2lMWACkc0XeODlN4xonpJhdyPPF+GLe89XDfLz9bJlSheQHv6rHxE48tKndalnjsJT1S5bz0VpCcoCGOk1dTIXrSzmf1oYY/R"
                         "ubHLR8wnNn6srYipe2XlzbFOvqaI6zlPMrAYqQzXsspF3aGmwArmU85lIujqOq8nBncx0dzV73Vct+CfgwsmSEF7692LehcX1Twg5Tg0gAMdRrn4N9HWgFx8f+4roX4CSgkBI+lSk"
                         "yNp+dZKj3vorAwNlNjiOTD3Q2VIhDkTKsdKAAolREXKo3JML4lRx5z2zmaM9EKUW22lZE+ONGkXhMR3ZkKORdnt/egkTIfpxLM3VNB8QmsiHIej5F0aBNDxAIKNXiiWIuVaTegYSj"
                         "sVTJyS0YDu3prPrZjoz+zrmx5UiKBM+HZdcn4wKiwaKzcshax6AU9rInLBcNSvlopVDYOK+O8/YjSd7ZcxcArg+pvOAjGISiMfhGQBToIFKlChEBe76Sz5L5lAwELDuYl54iQs5XI"
                         "RYRUNX+IIsVAWVfCHNZDaoZI/WprRWwlZShrR8qhXa0ZwJlTaKskqMKTEfGaNGLgNKgdWkM5yF+lsHkTHUlW9ZwNbhmLrUCNZsI5xpEDd3cKuL6I5SevskRATdf07YECpmAQUNYFw"
                         "DMYArDKze7Q/M7EP0NiBPmNpfBHOla+TJ56xLU1Uc/uRh2MNh2/tbtevDqEyCnQEq9wM3xwS6n5pABY3jzTB5i8fKXHJj9vNs+cvsL572FbRg+Azas8uqdwsgAg63f1y6oFeGH286"
                         "R8HoRXgG5+C/EvyLyGubG1pXIayNQgB2xWGCa3Ye7Wd+/Fbu+GVE22dQV/pyY5svXfwGKQCEwN2J+rYAVENqAE1j53QrGcsR+QFgMyApAPrDyuwfwD4nzOUEakPntAAAAAElFTkSu"
                         "QmCC\" style=\"vertical-align: middle;\" /></a><a href=\"button://show\"><span style=\" text-decoration: underline; color:#0000ff;\">"
                         "%2</span></a></p></body></html>";


PacmanErrorBalloonTip::PacmanErrorBalloonTip(SystemTrayIcon *ti) : QObject() {
    connect(ti,SIGNAL(anchorClicked(const QUrl &)),this,SIGNAL(showErrorsRequested()));
    connect(ti,SIGNAL(messageWindowDestroyed()),this,SLOT(deleteLater()));
    this->ti = ti;
    message_str = QString(error_str).arg(tr("There were the errors during the last checking...")).arg(tr("Details..."));
}

void PacmanErrorBalloonTip::show() {
    ti->showMessage(tr("Errors"),message_str,SystemTrayIcon::Critical,3000);
}
