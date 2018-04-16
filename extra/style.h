#ifndef STYLE_H
#define STYLE_H

#define OKBTN_STYLE     "QToolButton{background-color: qlineargradient(spread:pad, x1:0, y1:0.5, x2:1, y2:0.5, stop:0 rgb(220,195,145), stop:1 rgb(220,195,145)); border-radius:5px;color: rgb(255, 255, 255);}"  \
                        "QToolButton:hover{background-color:qlineargradient(spread:pad, x1:0, y1:0.5, x2:1, y2:0.5, stop:0 rgb(185,150,85), stop:1 rgb(185,150,85));}"   \
                        "QToolButton:disabled{background-color:qlineargradient(spread:pad, x1:0, y1:0.5, x2:1, y2:0.5, stop:0 rgb(83,90,109), stop:1 rgb(70,76,93));color: rgb(40,45,57);}"

#define CANCELBTN_STYLE "QToolButton{background:transparent;color:rgb(220,195,145);border:1px solid rgb(220,195,145);border-radius:3px;}"  \
                        "QToolButton:hover{background-color:rgb(185,150,85);color:white;}"

#define CLOSEBTN_STYLE  "QToolButton{background-image:url(:/ui/wallet_ui/closeBtn.png);background-repeat: no-repeat;background-position: center;border-style: flat;}"   \
                        "QToolButton:hover{background-image:url(:/ui/wallet_ui/closeBtn_hover.png);"

#define CONTAINERWIDGET_STYLE   "#containerwidget{background-color:rgb(244,244,242);border-radius:10px;}"  \
                                "QLabel{color:rgb(220,195,145);}"



#endif // STYLE_H
