/*
 * Copyright (C) 2014 Red Hat
 *
 * This file is part of openconnect-gui.
 *
 * openconnect-gui is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vpninfo.h"
#include "config.h"
#include "dialog/MyCertMsgBox.h"
#include "dialog/MyInputDialog.h"
#include "dialog/MyMsgBox.h"
#include "dialog/mainwindow.h"
#include "gtdb.h"
#include "logger.h"
#include "server_storage.h"

#include <QDir>
#include <QHash>
#include <QUrl>

#include <cstdarg>
#include <cstdio>

static const char* OCG_PROTO_GLOBALPROTECT = "gp";
static const char* OCG_PROTO_FORTINET = "fortinet";

static int last_form_empty;

static void stats_vfn(void* privdata, const struct oc_stats* stats)
{
    VpnInfo* vpn = static_cast<VpnInfo*>(privdata);
    const char* cipher;
    QString dtls;

    cipher = openconnect_get_dtls_cipher(vpn->vpninfo);
    if (cipher != nullptr) {
        dtls = QLatin1String(cipher);
    }

    vpn->m->updateStats(stats, dtls);
}

// privdata is set by the caller to be of type VpnInfo
// Access as: VpnInfo* vpn = static_cast<VpnInfo*>(privdata);
static void progress_vfn(void* privdata, int level, const char* fmt, ...)
{
    char buf[512];
    size_t len;
    va_list args;

    /* don't spam */
    if (level == PRG_TRACE)
        return;

    buf[0] = 0;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    len = strlen(buf);
    if (buf[len - 1] == '\n')
        buf[len - 1] = 0;
    Logger::instance().addMessage(buf);
}

static int process_auth_form(void* privdata, struct oc_auth_form* form)
{
    VpnInfo* vpn = static_cast<VpnInfo*>(privdata);
    bool ok;
    QString text;
    struct oc_form_opt* opt;
    QStringList gitems;
    QStringList ditems;
    int i, idx;

    if (form->banner) {
        Logger::instance().addMessage(QLatin1String(form->banner));
    }

    if (form->message) {
        Logger::instance().addMessage(QLatin1String(form->message));
    }

    if (form->error) {
        Logger::instance().addMessage(QLatin1String(form->error));
    }

    int empty = 1;
    if (form->authgroup_opt) {
        struct oc_form_opt_select* select_opt = form->authgroup_opt;

        for (i = 0; i < select_opt->nr_choices; i++) {
            ditems << select_opt->choices[i]->label;
            gitems << select_opt->choices[i]->name;
        }

        if (select_opt->nr_choices == 1) {
            openconnect_set_option_value(&select_opt->form,
                select_opt->choices[0]->name);
        } else if (gitems.contains(vpn->ss->get_groupname())) {
            openconnect_set_option_value(&select_opt->form,
                vpn->ss->get_groupname().toUtf8().data());
        } else {
            {
                MyInputDialog dialog(vpn->m,
                    QLatin1String("Auth group selection"),
                    QLatin1String(select_opt->form.label),
                    ditems);
                dialog.show();
                ok = dialog.result(text);
            }

            if (!ok)
                goto fail;

            idx = ditems.indexOf(text);
            if (idx == -1)
                goto fail;

            openconnect_set_option_value(&select_opt->form,
                select_opt->choices[idx]->name);
            text = QLatin1String(select_opt->choices[idx]->name);

            Logger::instance().addMessage(QLatin1String("Saving group: ") + text);
            vpn->ss->set_groupname(text);
        }

        if (vpn->authgroup_set == 0) {
            vpn->authgroup_set = 1;
            return OC_FORM_RESULT_NEWGROUP;
        }
    }

    for (opt = form->opts; opt; opt = opt->next) {
        text.clear();
        if (opt->flags & OC_FORM_OPT_IGNORE)
            continue;

        if (opt->type == OC_FORM_OPT_SELECT) {
            QStringList items;
            struct oc_form_opt_select* select_opt = reinterpret_cast<oc_form_opt_select*>(opt);

            Logger::instance().addMessage(QString::fromUtf8("Select form: ") + QString::fromUtf8(opt->name));

            if (select_opt == form->authgroup_opt) {
                continue;
            }

            for (i = 0; i < select_opt->nr_choices; i++) {
                items << select_opt->choices[i]->label;
            }

            {
                MyInputDialog dialog(vpn->m,
                    QLatin1String("Form selection"),
                    QString::fromUtf8(opt->label), items);

                dialog.set_banner(QString::fromUtf8(form->banner), QString::fromUtf8(form->message));
                dialog.show();
                ok = dialog.result(text);
            }

            if (!ok)
                goto fail;

            idx = ditems.indexOf(text);
            if (idx == -1)
                goto fail;

            openconnect_set_option_value(opt, select_opt->choices[idx]->name);
            empty = 0;
        } else if (opt->type == OC_FORM_OPT_TEXT) {
            Logger::instance().addMessage(QString::fromUtf8("Text form: ") + QString::fromUtf8(opt->name));

            if (vpn->form_attempt == 0
                && vpn->ss->get_username().isEmpty() == false
                && vpn->is_username_form_option(form, opt)) {
                openconnect_set_option_value(opt,
                    vpn->ss->get_username().toUtf8().data());
                empty = 0;
                continue;
            }

            do {
                MyInputDialog dialog(vpn->m,
                    QLatin1String("Username input"),
                    QString::fromUtf8(opt->label),
                    QLineEdit::Normal);

                dialog.set_banner(QString::fromUtf8(form->banner), QString::fromUtf8(form->message));
                dialog.show();
                ok = dialog.result(text);

                if (!ok)
                    goto fail;
            } while (text.isEmpty());

            if (vpn->is_username_form_option(form, opt)) {
                vpn->ss->set_username(text);
            }

            openconnect_set_option_value(opt, text.toUtf8().data());
            vpn->form_attempt++;
            empty = 0;
        } else if (opt->type == OC_FORM_OPT_PASSWORD) {
            Logger::instance().addMessage(QString::fromUtf8("Password form: ") + QString::fromUtf8(opt->name));

            if (vpn->form_pass_attempt == 0
                && vpn->ss->get_password().isEmpty() == false
                && vpn->is_password_form_option(form, opt)
               ) {
                openconnect_set_option_value(opt,
                    vpn->ss->get_password().toUtf8().data());
                empty = 0;
                continue;
            }

            MyInputDialog dialog(vpn->m,
                QLatin1String("Password input"),
                QString::fromUtf8(opt->label),
                QLineEdit::Password);

            dialog.set_banner(QString::fromUtf8(form->banner), QString::fromUtf8(form->message));
            dialog.show();
            ok = dialog.result(text);

            if (!ok)
                goto fail;

            if (vpn->is_password_form_option(form, opt)
                && (vpn->password_set == 0 || vpn->form_pass_attempt != 0)) {
                vpn->ss->set_password(text);
                vpn->password_set = 1;
            }
            openconnect_set_option_value(opt, text.toUtf8().data());
            vpn->form_pass_attempt++;
            empty = 0;
        } else {
            Logger::instance().addMessage(QLatin1String("unknown type ") + QString::number((int)opt->type));
        }
    }

    /* prevent infinite loops if the authgroup requires certificate auth only */
    if (last_form_empty && empty) {
        return OC_FORM_RESULT_CANCELLED;
    }
    last_form_empty = empty;

    return OC_FORM_RESULT_OK;
fail:
    return OC_FORM_RESULT_CANCELLED;
}

static int validate_peer_cert(void* privdata, const char* reason)
{
    VpnInfo* vpn = static_cast<VpnInfo*>(privdata);
    unsigned char* der = nullptr;
    int der_size = openconnect_get_peer_cert_DER(vpn->vpninfo, &der);
    if (der_size <= 0) {
        Logger::instance().addMessage(QObject::tr("Peer's certificate has invalid size!"));
        return -1;
    }

    const char* hash = openconnect_get_peer_cert_hash(vpn->vpninfo);
    if (hash == nullptr) {
        Logger::instance().addMessage(QObject::tr("Error getting peer's certificate hash"));
        return -1;
    }

    gnutls_datum_t raw;
    raw.data = der;
    raw.size = der_size;

    gtdb tdb(vpn->ss);
    int ret = gnutls_verify_stored_pubkey(reinterpret_cast<const char*>(&tdb),
        tdb.tdb, "", "", GNUTLS_CRT_X509, &raw, 0);

    char* details = openconnect_get_peer_cert_details(vpn->vpninfo);
    QString dstr;
    if (details != nullptr) {
        dstr = QString::fromUtf8(details);
        free(details);
    }

    bool save = false;

    // If the existing server PIN uses an older algorithm than the current
    // we use, force save to update it to the latest.
    if (vpn->ss->server_pin_algo_is_legacy() == true)
        save = true;

    if (ret == GNUTLS_E_NO_CERTIFICATE_FOUND) {
        Logger::instance().addMessage(QObject::tr("peer is unknown"));

        QString hostInfoStr = QObject::tr("Host: ") + vpn->ss->get_server_gateway() + QObject::tr("\n") + hash;
        MyCertMsgBox msgBox(
            vpn->m,
            QObject::tr("This server's certificate cannot be validated with a trusted authority.<br><br>"
                        "<b>There is no guarantee that the server is the computer you think it is.</b>\n\n"
                        "If the information provided below is valid and you use another way to validate it, "
                        "hit 'Accurate information' to remember it and to carry on connecting.\n"
                        "If you cannot validate this information, hit <b>Cancel</b> to abandon the connection."),
            hostInfoStr,
            QObject::tr("Accurate information"),
            dstr);
        msgBox.show();
        if (msgBox.result() == false) {
            return -1;
        }

        save = true;
    } else if (ret == GNUTLS_E_CERTIFICATE_KEY_MISMATCH) {
        Logger::instance().addMessage(QObject::tr("peer's key has changed!"));

        MyCertMsgBox msgBox(vpn->m,
            QObject::tr("This peer is known and associated with a different key."
                        "It may be that the server has multiple keys "
                        "or you are (or were in the past) under attack. "
                        "Do you want to proceed?"),
            QObject::tr("Host: %1\n%2").arg(vpn->ss->get_server_gateway()).arg(hash),
            QObject::tr("The key was changed by the administrator"),
            dstr);
        msgBox.show();
        if (msgBox.result() == false) {
            return -1;
        }

        save = true;
    } else if (ret < 0) {
        QString str = QObject::tr("Could not verify certificate: ");
        str += gnutls_strerror(ret);
        Logger::instance().addMessage(str);
        return -1;
    }

    if (save != false) {
        Logger::instance().addMessage(QObject::tr("saving peer's public key"));
        ret = gnutls_store_pubkey(reinterpret_cast<const char*>(&tdb), tdb.tdb,
            "", "", GNUTLS_CRT_X509, &raw, 0, 0);
        if (ret < 0) {
            QString str = QObject::tr("Could not store certificate: %1").arg(gnutls_strerror(ret));
            Logger::instance().addMessage(str);
        } else {
            vpn->ss->save();
        }
    }
    return 0;
}

static int lock_token_vfn(void* privdata)
{
    VpnInfo* vpn = static_cast<VpnInfo*>(privdata);

    openconnect_set_token_mode(vpn->vpninfo,
        (oc_token_mode_t)vpn->ss->get_token_type(),
        vpn->ss->get_token_str().toUtf8().data());

    return 0;
}

static int unlock_token_vfn(void* privdata, const char* newtok)
{
    VpnInfo* vpn = static_cast<VpnInfo*>(privdata);

    vpn->ss->set_token_str(newtok);
    vpn->ss->save();
    return 0;
}

static QByteArray native_path(const QString& path) {
    return QDir::toNativeSeparators(path).toUtf8();
}

static void setup_tun_vfn(void* privdata)
{
    VpnInfo* vpn = static_cast<VpnInfo*>(privdata);

    QByteArray vpncScriptFullPath;
    QByteArray interface_name;

    if (!vpn->ss->get_vpnc_script_filename().isEmpty())
        vpncScriptFullPath = native_path(vpn->ss->get_vpnc_script_filename());
    else if (QDir::isAbsolutePath(DEFAULT_VPNC_SCRIPT))
        vpncScriptFullPath = native_path(QString(DEFAULT_VPNC_SCRIPT));
    else
        vpncScriptFullPath = native_path(QCoreApplication::applicationDirPath()
                                         + "/" + QString(DEFAULT_VPNC_SCRIPT));

    if (!vpn->ss->get_interface_name().isEmpty())
        interface_name = vpn->ss->get_interface_name().toUtf8();
#ifdef _WIN32
#if ! (OPENCONNECT_API_VERSION_MAJOR == 5 && OPENCONNECT_API_VERSION_MINOR == 9)
#error "This probably has been fixed in openconnect in API version >= 5.9 and this workaround is not required anymore."
#endif
    else {
        //generate a "unique" interface name if no interface name was specified.
        //Normally libopenconnect will use the server name as interface name to force
        //switching to wintun on windows.
        //But, if TAP or openvpn's wintun adapters are already present on the system
        //and no interface name is specified, these are used instead.
        //So, use this "unique" interface name as a workaround to force wintun from openconnect.
        //See openconnect-gui#357 (comment 1758999655) and openconnect#699
        interface_name = vpn->generateUniqueInterfaceName();

        Logger::instance().addMessage(QObject::tr("Using generated interface name %1").arg(QString::fromUtf8(interface_name)));
    }
#endif

    int ret = openconnect_setup_tun_device(vpn->vpninfo,
                                           vpncScriptFullPath.constData(),
                                           interface_name.constData());
    if (ret != 0) {
        vpn->last_err = QObject::tr("Error setting up the TUN device");
        //FIXME: ???        return ret;
    }

    vpn->logVpncScriptOutput();
}

static inline int set_sock_block(int fd)
{
#ifdef _WIN32
    unsigned long mode = 0;
    return ioctlsocket(fd, FIONBIO, &mode);
#else
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
#endif
}

VpnInfo::VpnInfo(QString name, StoredServer* ss, MainWindow* m)
{
    this->vpninfo = openconnect_vpninfo_new(name.toUtf8().data(), validate_peer_cert, nullptr,
        process_auth_form, progress_vfn, this);
    if (this->vpninfo == nullptr) {
        throw std::runtime_error("initial setup fails");
    }

    //get loglevel preference from profile
    int loglevel = ss->get_log_level();

    if (loglevel == -1) {
        //-1 means use application default
        loglevel = m->get_log_level();
    }

    openconnect_set_loglevel(vpninfo, loglevel);

    this->cmd_fd = openconnect_setup_cmd_pipe(vpninfo);
    if (this->cmd_fd == INVALID_SOCKET) {
        Logger::instance().addMessage(QObject::tr("invalid socket"));
        throw std::runtime_error("pipe setup fails");
    }
    set_sock_block(this->cmd_fd);

    this->last_err = "";
    this->ss = ss;
    this->m = m;
    authgroup_set = 0;
    password_set = 0;
    form_attempt = 0;
    form_pass_attempt = 0;
    openconnect_set_stats_handler(this->vpninfo, stats_vfn);
    if (ss->get_token_str().isEmpty() == false) {
        openconnect_set_token_callbacks(this->vpninfo, this, lock_token_vfn, unlock_token_vfn);
        openconnect_set_token_mode(this->vpninfo,
            (oc_token_mode_t)ss->get_token_type(),
            ss->get_token_str().toUtf8().data());
    }

    //openconnect_set_protocol() checks the token with UTF8CHECK
    openconnect_set_protocol(vpninfo, ss->get_protocol_name().toUtf8().data());

    openconnect_set_setup_tun_handler(vpninfo, setup_tun_vfn);
}

VpnInfo::~VpnInfo()
{
    if (vpninfo != nullptr) {
        openconnect_vpninfo_free(vpninfo);
    }

    delete ss;
}

void VpnInfo::setUrl(const QUrl& url)
{
    this->mUrl = url;

    if (mUrl.scheme().isEmpty()) {
        mUrl.setScheme(QStringLiteral("https"));
    }

    openconnect_parse_url(this->vpninfo, mUrl.url().toUtf8().constData());
}

int VpnInfo::connect()
{
    int ret;
    QString cert_file, key_file;
    QString ca_file;

    //disable DTLS early on if specified on profile
    if (this->ss->get_disable_udp() == true) {
        ret = openconnect_disable_dtls(vpninfo);
        if (ret != 0) {
            this->last_err = QObject::tr("Error disabling DTLS (%1)").arg(ret);
            return ret;
        }
    }

    cert_file = ss->get_cert_file();
    ca_file = ss->get_ca_cert_file();
    key_file = ss->get_key_file();

    if (key_file.isEmpty() == true)
        key_file = cert_file;

    if (cert_file.isEmpty() != true) {
        openconnect_set_client_cert(vpninfo, cert_file.toUtf8().data(),
            key_file.toUtf8().data());
    }

    if (ca_file.isEmpty() != true) {
        openconnect_set_system_trust(vpninfo, 0);
        openconnect_set_cafile(vpninfo, ca_file.toUtf8().data());
    }

#ifdef Q_OS_WIN32
    const QString osName{ "win" };
#elif defined Q_OS_MACOS
    const QString osName{ "mac-intel" };
#elif defined Q_OS_LINUX
    const QString osName = QString("linux%1").arg(QSysInfo::buildCpuArchitecture() == "i386" ? "" : "-64").toStdString().c_str();
#elif defined Q_OS_FREEBSD
    const QString osName = QString("freebsd%1").arg(QSysInfo::buildCpuArchitecture() == "i386" ? "" : "-64").toStdString().c_str();
#else
#error Define OS string of other platforms...
#endif
    openconnect_set_reported_os(vpninfo, osName.toStdString().c_str());

    ret = openconnect_obtain_cookie(vpninfo);
    if (ret != 0) {
        this->last_err = QObject::tr("Authentication error; cannot obtain cookie");
        return ret;
    }

    ret = openconnect_make_cstp_connection(vpninfo);
    if (ret != 0) {
        this->last_err = QObject::tr("Error establishing the CSTP channel");
        return ret;
    }

    if (this->ss->get_disable_udp() != true) {
        ret = openconnect_setup_dtls(vpninfo, ss->get_dtls_reconnect_timeout());

        if (ret != 0) {
            this->last_err = QObject::tr("Error setting up DTLS (%1)").arg(ret);

            //FIXME: this call we possibly fail since CSTP is already connected, but will try it anyway and fail if we can't
            //we don't have any other way in the openconnect library to disable it at this stage
            ret = openconnect_disable_dtls(vpninfo);

            if (ret != 0) {
                this->last_err += QObject::tr(". Cannot disable it (%1)").arg(ret);
            }

            return ret;
        }
    }

    return 0;
}

void VpnInfo::mainloop()
{
    while (true) {
        int ret = openconnect_mainloop(vpninfo,
            ss->get_reconnect_timeout(),
            RECONNECT_INTERVAL_MIN);
        if (ret != 0) {
            this->last_err = QObject::tr("Disconnected");
            logVpncScriptOutput();
            break;
        }
    }
}

void VpnInfo::get_info(QString& dns, QString& ip, QString& ip6)
{
    const struct oc_ip_info* info;
    int ret = openconnect_get_ip_info(this->vpninfo, &info, nullptr, nullptr);
    if (ret == 0) {
        if (info->addr) {
            ip = info->addr;
            if (info->netmask) {
                ip += "/";
                ip += info->netmask;
            }
        }
        if (info->addr6) {
            ip6 = info->addr6;
            if (info->netmask6) {
                ip6 += "/";
                ip6 += info->netmask6;
            }
        }

        dns = info->dns[0];
        if (info->dns[1]) {
            dns += ", ";
            dns += info->dns[1];
        }
        if (info->dns[2]) {
            dns += " ";
            dns += info->dns[2];
        }
    }
    return;
}

void VpnInfo::get_cipher_info(QString& cstp, QString& dtls)
{
    const char* cipher = openconnect_get_cstp_cipher(this->vpninfo);
    if (cipher != nullptr) {
        cstp = QLatin1String(cipher);
    }
    cipher = openconnect_get_dtls_cipher(this->vpninfo);
    if (cipher != nullptr) {
        dtls = QLatin1String(cipher);
    }
}

SOCKET VpnInfo::get_cmd_fd() const
{
    return cmd_fd;
}

void VpnInfo::reset_vpn()
{
    openconnect_reset_ssl(vpninfo);
    form_pass_attempt = 0;
    password_set = 0;
    authgroup_set = 0;
    form_attempt = 0;
}

bool VpnInfo::get_minimize() const
{
    return ss->get_minimize();
}

void VpnInfo::logVpncScriptOutput()
{
    /* now read %temp%\\vpnc.log and post it to our log */
    QString tfile = QDir::tempPath() + QLatin1String("/vpnc.log");
    QFile file(tfile);
    if (file.open(QIODevice::ReadOnly) == true) {
        QTextStream in(&file);

        QString bannerMessage;
        bool processBannerMessage = false;

        while (!in.atEnd()) {
            const QString line{ in.readLine() };
            Logger::instance().addMessage(line);

            if (line == QLatin1String("--------------------- BANNER ---------------------")) {
                processBannerMessage = true;
                continue;
            }
            if (line == QLatin1String("------------------- BANNER end -------------------")) {
                processBannerMessage = false;
                continue;
            }
            if (processBannerMessage) {
                bannerMessage += line + "\n";
            }
        }
        file.close();
        if (file.remove() != true) {
            Logger::instance().addMessage(QLatin1String("Could not remove ") + QDir::toNativeSeparators(tfile) + ": " + file.errorString());
        }

        if (ss->get_batch_mode() != true && bannerMessage.isEmpty() == false) {
            // TODO: msgbox title; e.g. Accept/Continue + Disconnect on buttons
            MyMsgBox msgBox(this->m,
                bannerMessage,
                QString(""),
                QString("Accept"));
            msgBox.show();
            if (msgBox.result() == false) {
                this->m->on_disconnectClicked();
            }
        }
    } else {
        Logger::instance().addMessage(QLatin1String("Could not open ") + QDir::toNativeSeparators(tfile) + ": " + file.errorString());
    }
}

QByteArray VpnInfo::generateUniqueInterfaceName()
{
    //generate a hash from server_gateway (as input) and username
    size_t uhash = qHash(this->ss->get_server_gateway() + this->ss->get_username(), 0);
    QString hash = QString::number(uhash, 16);
    QString host = mUrl.host();

#ifdef _WIN32
    /* Reduce host so the total length (host + underscore + hash) fits in openconnect's buffer size */
    qsizetype maxHostLen = OC_IFNAME_MAX_LENGTH - 1 - hash.length();
    host.truncate(maxHostLen);
#endif /* _WIN32 */
    return host.append("_").append(hash).toUtf8();
}

bool VpnInfo::is_username_form_option(struct oc_auth_form* form, struct oc_form_opt* opt)
{
    bool ret = false;
    QByteArray protocolName = this->ss->get_protocol_name().toUtf8();

    if (form && opt && opt->name) {
        if (strcasecmp(OCG_PROTO_GLOBALPROTECT, protocolName.constData()) == 0) {
            ret = ( (strcasecmp(form->auth_id, "_login") == 0) && (strcasecmp(opt->name, "user") == 0) );
        }
        else {
            ret = ( strcasecmp(opt->name, "username") == 0 );
        }
    }

    return ret;
}

bool VpnInfo::is_password_form_option(struct oc_auth_form* form, struct oc_form_opt* opt)
{
    bool ret = false;
    QByteArray protocolName = this->ss->get_protocol_name().toUtf8();

    if (form && opt && opt->name) {
        if (strcasecmp(OCG_PROTO_GLOBALPROTECT, protocolName.constData()) == 0) {
            ret = ( (strcasecmp(form->auth_id, "_login") == 0) && (strcasecmp(opt->name, "passwd") == 0) );
        }
        else if (strcasecmp(OCG_PROTO_FORTINET, protocolName.constData()) == 0) {
            ret = ( (strcasecmp(form->auth_id, "_login") == 0) && (strcasecmp(opt->name, "credential") == 0) );
        }
        else {
            ret = ( strcasecmp(opt->name, "password") == 0 );
        }
    }

    return ret;
}
