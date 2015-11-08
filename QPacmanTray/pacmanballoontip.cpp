/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanballoontip.h"
#include "systemtrayicon.h"

#ifndef max
#define min(a,b) ((a<b)?a:b)
#endif

static const char * message_str_part1 = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                                        "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
                                        "p, li { white-space: pre-wrap; }"
                                        "</style></head><body style=\" font-style:normal;\">";
static const char * message_str_packg = "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%1</p>";
static const char * message_str_part2 = "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                        "<a href=\"button://update\"><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAABHNCSVQICAgIfAhkiAA"
                                        "AAAlwSFlzAAAFMQAABTEBt+0oUgAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAABJASURBVHjarVoJeFRVlj5vrXpVqSWVhOyVBYgRUNoASWSTgNoqdNN2ty24"
                                        "xFEbHVR6HL62W7vHGVv91B57XGAUp1FbG2kUhzRiqyyygwqyD9JBQBKyVPZUVVLLe6/ee3POq1dQ5gt8OMMJ/3dfbr177n/OPefce1MwhmHApRSGYVhsBIQNwVndGkJGqDifDpdQ+"
                                        "P8HUcZAGdpHxJ977qUbZsyY8RtN0wQgKzhO3bZt2zOPPvrwenwlTuPOo+u7C467aKAQQRYhIuyWA5i0z1lETn39A/8WjSJPS+iZ+ugzBDtEH2/pEq3xzHfhxH7H0BARWbNn/6h669Y"
                                        "9qzIzMwvI45bnU4RsiqLkodchJfRMfVZYMemrRTpIF+kk3QiR5rqkK2B5xomoWLPmo3XffNOuhMODxttvv78W+4pSK2HFfukdd9z/NoaPkRJ6pj76DCGkeb6IdJAu0km6aQ5rLvYiu"
                                        "F10DnC33nrH5Y8//sSmsrJCr8NhB5IpU2rn/Oxn8/+6evWqvwFA1FqhTIbhvCx7zok6owGTqxbO+cOU2VdMGyPZRDHWeqa9NbKTKZ88pXqOy+UEhJCbm/mDo0dPTnvqqSeue++9dw7"
                                        "TULiQWPF2MeFDS1/5wQebN/zwhzNz01dvw4adLTfeeM1dkydPExYt+ufbioqKp/p8Wf7C0V7hs75P4YvgFuiKt0OxoxyKJD/4bDkgsnaIqoPQpwagK9YOWUIujPNUw1WeKbDxoz2dc"
                                        "+fO+j4ANKJ++VIZQMvunzx51gMNDe8vzs72QErC4QgEAl0xny/T7nI5GIWPwsq2ZbCu610Y762CMtdIKHcXg0eQIMHEQAMZBIYHO+cEO+OEzng/tA10wbHgYegaPAPdO7FgvXxg4U"
                                        "CTcgq5qZAmTx6bN25O/oLFXwQa7nlgzCvfyQCKWWJd88Ybq9+6886b82AYaeh4B5a3Pg9XeifCeN84MPg4gKEBiznJswhsOZbAAYcqeYaj381WYjOgfTAE+zv3QnP4WBNrwN0rag5u"
                                        "A0vu31/9eI6j9Mkxnhr4sntd4Yvjt7cPa4BFlk2Fr4Fi9dspwaqrpy5qaGj4eXa2F1KSQEc9d+w3cCi+F2YVXg8JbhAMIk5kGYu4SZZJkgf2vEY5GDd8Ew7AZ4H1qq4pi/486cAf79"
                                        "478bceu++p2SNvhu2tO6A/0lzyn1ftPgMo/DClUlqw4MGy5ctf6cXnQdp4KNwR9iuuuKrgpZeW/tTjyQBN04FE0RR4cM980OwqVOdPhoDSYpLjUmDTWmAvaBQZNIDTeewCzCr+ibCn"
                                        "ffNrd+65qj7D5ps8rfhGYLkE6GDQD5OeA+metyNK9+37aocg8LB9+/bXfvGL+/6EfbGqqurKV19dvrqycmQ2x7GQksWfLYB+vgeK8oohqoeBTSNNYNMMYeECRiEE83P+rFGC4YD+WBh"
                                        "8kgtEQTff+bL9IPSHvyn/r+ovTw9dAQ7he+21Pz9dWJiX7XDYIDv75n+pqan5+bp1a9+fM2fu7RUVZT4yU9eT3n/58O/h63AjjKqogIAcILImIcYMDcb83YYFjGpAl9IJ3XIvhBIRU"
                                        "HUVXLwDsm1eKHXkY2VygcDyOGaI4WwcBJGFiD4AcZX08aCDBtFQ4qwH+fRdcdq0GeNra2vn2O0ikjSA6n1FRXnewoUPLKI+eo36SU4EG2H10Xdg/KQJ0BYPWHFtTWyS58DBOeBA+H/g"
                                        "YPgIDCd2JCQi8Xx7NlyTUwO5tqzkSp01gkm21AcMMKiTfBdulW2p8xOffgSor7/v7sxMj0geThdJEs02vf+Z7f8K/sJSaJLP0EQIK0QQds4GcV2GT4LbIKJFIQNLZrbsBluEA7cmAQ"
                                        "Y7yA4ZVEmDIIQgqgzApo6tMMY1GsZ6KqkqmfqYlF5sGUu/aiQg2qumjiRGegglFi6sX1pfv9C9YMF915aXF9G44QTj8HM41doI3tI8iGpxVHvOAGqDiUHYFz5IPoMyuRD4vTqo3YqqG"
                                        "yIblTVOlFgAF4CYYcBlFcUQzh+EOMSgaaAJ+nFfqM6eSAmeJE4/LJPSTW4EzonZbAlrtQYilkgkvn7zzaXP3HLL7KdWrHi/IxyOmiEzFCsPvAWe8izo1LohhgZECQlqY1hSdQybw0h"
                                        "V0L8XHqWxB+RBI0P9VZO/13biTwHB31/5e/6IR/XsL4esveNAPsKA55QEeUw2OFgBFFyxE6GvQUkoEE/ICGxVBaL4HFXjoKF+t98mE+fhqhBnHaSKETMaGjb+e21tlQRDZOqy8ZBVm"
                                        "w0dQq+5tJzBATZIWtQDSsAIJcLaeGZkPNIY7GRFmHr8oZ5uSJPp06ePxRl3CYLkFUUJIlNPgadagJDYbxZJHVHqGA0uwU3M8B/1UnEwgNEjMDjYNXrFlP0n01cAjKTQ0gwg2idOrBk"
                                        "sKyu1U71Px/ojH4I32wFtXBeoCQ00RYOYEVNjejzap/ZHkHzIbxvRo3XH2rg8bv5Q8iQ7duz4ym631eq6oqo1AfBMxDBxqiBhUjuxahWIBYDOAEVXcEVwFTQFgUGmyqDRnIZ2wRsZ"
                                        "gxCvvfamqzMyHAyFTLrsbd4LklOChIo132BBYZSgqicC6B971IjS0sa9qiMqO6LbGm/pPADnkY0bNx7//tLqXVKVVidyPKhI0idkgY1zmww0PQG6RYeeQmoI+tQ+GC3mgJE4t5HxDIp"
                                        "lCJd2ps8pLS2v5XnuW5UnEhmEiDKAScQDp/G4IonEgH3gFKOzg7jxxGRdFsgAu8bFFJvxGVxAbjs47cr86fY6XtcAEgzwfBaQ6EhBM4kYIGO4DGgDEFYHQKHAYgFUWYWuwxGNqWNor"
                                        "gQRl5Yte/MHBQUFdWiLh+M4p6IoWZmZOZcNKacYe4Mgoyq7IAGvcZiwSr9iKD2YCBFWZ+IYqfEsm4uP9sTijMQegwuIR4TRDJ4/WkIdPIMeNZAoozBJn2sMhb0JO5oisW7aC8w49yY"
                                        "Km28ve/pXty5r37pw4T0fkgGZqsrUV1aOvSmZzISkoqHhQ/7h0XA7Zwen7oQwF2rSQafMi+joeQAs77woK7LK6SIG7QVk2dida6o/KnFUrp12lxzRljskN+OQJCgqKoDc3AKQHA7AP"
                                        "EHYwWazQ0FhIRpEzLhSOpw2Nweo0OwiA5wtLW1Sc3MHvijgABFbgoBWM5AudAbKFLOAhQhIuqTLotyE3UFElMgT2qJ9ypScUbH2WB8duQNwAdk7uznxu32/22CzOdSCgjKRCOP+A24"
                                        "3hZM1t+VIOaqCLGMyxxVsVSDOxJ0MYDFkoLs7CENFFAUy5KxhGRkc5Ei5oDNByMAfFtiwZUAMoRB0w1BAgjgbNTLhIkSSJN3jyQSXy0U1EcFAoKMXiaomUYWgqKZB6UKc6W3WTASeV"
                                        "2AYUdUEDAxEoacnBK2tuGnFNBiZNRJLXQZ4hUw2V83FWSGchgEGwTvZqCCKrnGb/U64oJCTxElut1cQcdU9XidEYwlowbloTppbVZMpPVSIcyqJB3bv3vJuY+ORDlyqHEHgHWiYZ9a"
                                        "sOWNHjMgT0y889DiuYBwc6z0EFW4B5Eh81Gk4vcNaARVTTxUZVvsy2CT/qOTK+NG+tstvOsns+3iUAeeTzEzfLWgAI9o58HpdEB4450vUR/mI0dGpbN78t68A9BA6NYqh3d3b27ONu"
                                        "JMBoS++2LaBjjgIu4WSkpLy3+bm5o2BNAmFYuD3j4BcVzEIdgy7weBIy/MyA6AgeUNgORWzJ/51pEsel1VknGjhSic1lZ358trTOgwjWPVmerxe87zj8rqhrb0VSRuQurMw2Jw8eez"
                                        "kunWr/gAAzYi4hT6ixFvJ14XoTft7TU9T08nPJ0+eOYb2gkRCMxM4Go2D0+mEy0ZUQo9yBka5R7t/HP3JzAZxzX8LDO4MeGXCVkVgMverLMPEJhSUw6nO9qzpW8oHNRkUVsctKsEBq"
                                        "xrwYOLZxSNy83MzUCfN7Ha6IRZVyCgwdKBLjZnAxAUA9iNaEQmEgdAQOmskRUMoCDll3c6dn+5SVVXneRYIZAAag9nfBRNHVwHPuGB87limiq+aX5moyOAZNiIkERVYNi4yXLwvNqh"
                                        "sb29U/NkjElcW+LlJRSX2ysxC24QRfuEf1F/6nQ7HU0VFpazdjuWzMB8CbX3AczwIPG8aIYo8EAfiYnk8ThwtrhoRZ4e50BMgGOwd6OhoDXNcUhmRp7atrcfMhfH+8eDkMmBMyVjbv"
                                        "cKCpVVaFRkR45G4kIQssCbUQz3N2oHu09r+UFMiwIf0qo7b8vM9pbsrKsZITqcDYz95aAu094LIC2SA5TQOurrawsQljaLJ73x3YhHhra+/e+a8eXc9m0gIJbqumfsCHrWBZZMrIUk2"
                                        "qKkZC0davgLRxkBfdADOtJ6Md+udLy9lnntdZDnNxvAqkRdNQ1gFDdPQQP2XkSX/JHDik2VllyH5DPPM73Y7YN++v1OIWodGg6oMyLJszimK2pn33lvxmzffXP6pVbYVAyVlQIq8ze"
                                        "PxFLz++solXm/udX19A2LKCzQBbSBEnkBK8/OzwF+SCyc6vgHJJkBcT0BnV5seCvX36Iy2szfe9UEH07Ivw3AlSozR431Szs2MwdZl+XJy8/LossSY5F0uCcOyE9rbe4g8nb2oJadRG"
                                        "aVnMwd9PpcSDHZ9es898xeFQqF2yl0yImUAhZLnjTfefcXtzptPSohoygBB4DB5JfTIt43Iy/NBfiHeC4Kd1nVPN0+RSjyOkA05FlepP8PhEhySC48KThzHUXEknXTnxpDsgs7O3jTy"
                                        "hrlxRiIx2gNSBlBLc2JoB1bde++8B6kCIXf9WwaMGJE/85lnXl3h9XolIm4ZQITRADvQaxhSVigRGNNTxf5cUEE1V4lEM/TUFdBqkfS5ym7q0tHQM2c6aAxVGiJIoFyj39EACqeUATo"
                                        "9I/n+2KOPLryzqyuwJWVA+pUyjh+c2rRp3YYUQVrmI0f2dfz61w9+cvjwwT5JEqgykGFnjSMHtGIIqBFcZo8HfJiQvgwPOOx2EFjerCp2m2CuYKY3AxPWCeFwmMgTUSKcWm10Bm96"
                                        "//Dh/X2PPLIQ59zXQecxchRx2rTpww3EkbgOd6XksPEhapctW/VHRYnD6tVvfbx79/ZNlDi4W17+4ovLHq2trRlBS2qFWUq5BVwRuwgOJ5JHYixnhkvSkwmdkhQ9LtNYIk9tyvu0u"
                                        "mZYff75nq6HH/7H58Lh4N+poEyZUnfdvHn1N9HV8/77592HfV8g+qiMphuQMkIEwLAuLBzX1tamWafJDmuzy3O7M2cuWfLaE1dfXZOD76ZXJiKPSK4aPWMz5AxDhIFWLN0AIk/EsQX"
                                        "Ys2dP90MP3f9EONy/xZrXRvMmOZVwbW3NR4kT7QPnu1KqNBDJh2hGi7hqPSdQ8Z6PP/7rxgkTqm6n06nL5SQjiFQqsS3yKQz9Jggs8gyCco/yjKc8QD0KfPRRw0aaw9pxo5YHiEsTk"
                                        "mdSZ65h9oFhNzMjddlP+7tp5fr1Wxr8/pJSiv/UniAIPBEzSZIBhOSQ4Q1gmKShFIqxmJxKYMyLpqbrr6/7MQA0IuJpc0OKkzGEMAtDxEiKbliSNtj59NPP/rSkpLT0XBVKVqVDhw5"
                                        "GaSQZI0l2uklRdaL7BIGeqf8s0Azj4MFDUTIm5QiC319aSnPQXAgmxSed0//1e2IOkT19+sw7UzezFLZv39Zyxx23Lff5fMzcuXMn4N9Wx02dOrW4qKhISDmPiLa2tqq7du1qwTg/"
                                        "unbt2v19fX3GypWrFsyYUVcMkFwd4ldXZ86xwgod/VJ9S2m74YYbrtm8eUfr8eNNxqlTLUZzc5tx/PhJpbKy8jEA+B6iAjEJMf+xxx7bHgwGjXRQH31mvVNBY2jsiROnlNbWgNHU1G"
                                        "qcPHnG2Lp1ZyvNRXNeyu+JtfXr15+YNWv6oiVLXtjQ398vUxj95S/vbGpsbKSKcQpxGnEMcQgv4m0wRKy+Q9Y7p2kMjV25csUm0oVGyi+99B8b6uqmLaK5aM5L+T0xY5W0AsTUvLy"
                                        "8h55//oXVWEGus75956z3eIT/8ccfXz50BaiPPkPw1rscjSUdL7zwwmrSSbppDmsu5pJ9T2xVA8W6+OAxu6P5kUcWf0Jxmh6rqXKLG9cADBHqS7uMgDUmiGX44OLFiykMlbQ/EGg0"
                                        "5yX9zx6WwgQaMmhNwiC0IaWN2viJEycOrlmzZlckEvEAAN3iQtRHnw0pz4nU1ZC6Lpr40H3gEkn6ncKHyEdkQFLI6IBFVjEu4aT/C2PkK6tS9p+LAAAAAElFTkSuQmCC\" width=\"24\" height=\"24\" align=\"middle\""
                                        "/>%1</a></p></body></html>";


PacmanBalloonTip::PacmanBalloonTip(const QStringList & packages,SystemTrayIcon *ti) : QObject() {
    message_str = message_str_part1;
    int count = min(packages.count(),6);
    for (int i=0;i<count;i++) {
        message_str+=QString(message_str_packg).arg(packages[i]);
    }
    if (packages.count() > 6) message_str+=QString(message_str_packg).arg("...");
    message_str+=QString(message_str_part2).arg(tr("Update..."));

    this->ti = ti;
    connect(ti,SIGNAL(anchorClicked(const QUrl &)),this,SIGNAL(updateRequested()));
    connect(ti,SIGNAL(messageWindowDestroyed()),this,SLOT(deleteLater()));
}

void PacmanBalloonTip::show() {
    ti->showMessage(tr("New packages are available:"),message_str,SystemTrayIcon::Information,3000);
}
