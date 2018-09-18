[![Build Status](https://travis-ci.com/opensoft/proofbase.svg?branch=develop)](https://travis-ci.com/opensoft/proofbase)

Base of Proof
=============
Basic part of proof framework: Core and Network modules. As all other parts of proof is heavily based on Qt

## ProofCore
Contains non-gui, non-network common stuff that is used across the Proof framework.

#### ProofObject
Basic class for almost all Proof classes (replaces QObject in fact). Additionally to being QObject it can:
 * call methods in thread where this objects belongs too
 * keep track of being non-dirty (i.e. default constructed)

#### Settings/SettingsGroup
Wrapper on top of QSettings. Stores data in .conf files (INI-format), can have common conf file for all proof-based apps (`proof-common.conf`).

#### CoreApplication
Wrapper on top of QCoreApplication. Main entry point for any proof-based non-gui app. Doesn't provide getter for QCoreApplication instance that it creates internally (since it can be fetched via qApp) but provides next functionality:
 * migrations
 * language change
 * app settings
 * proofApp macros similar to qApp, but returns instance of Proof::CoreApplication
 * crash handler on linux with writing proof_crash_* files with stack trace if possible

#### Logging system
Installs custom logging handler that changes format of data that outputs to terminal. Also writes logs to day-based files and archives old proof-related logs after 24 hours.

#### ObjectsCache
Different types of caches for shared pointers for classes that provide create() method:
 * WeakObjectsCache - stores QWeakPointer
 * StrongObjectsCache - stores QSharedPointer
 * GuaranteedLifeTimeObjectsCache - stores QWeakPointer but also keeps shared pointer for predefined amount of time on Proof::Expirator

#### ErrorNotifier
Mechanism for sending errors to various sources (in-memory, emails, etc.). Core module contains only in-memory storage which will store anything that is either specifically registered via ErrorNotifier or sent to logging system with qWarning or higher.

## ProofNetwork
Contains network-related classes used across Proof framework and proof-based apps.

#### RestClient
Wrapper on top of QNetworkAccessManager for HTTP-based requests.
 * Moves itself to another thread in ctor.
 * Queues requests internally based on host where they are sent.
 * All RestClients use single QNetworkAccessManager internally.
 * Supports auth schemes based on WSSE, Basic, Bearer token.
 * Supports timeouts for requests.
 * All requests returns `CancelableFuture<QNetworkReply *>` which is filled when this request is effectively sent.

#### BaseRestApi
Base class for all API classes. Uses RestClient internally and provides protected interface for derived classes that returns `CancelableFuture<RestApiReply>` where `RestApiReply` is a simple structure with response from web service. Provides basic data unmarshallers for strings, ints and `NetworkDataEntity`.

#### NetworkDataEntity
Base class for all DTO classes. Designed to be used in shared pointers. Non-thread safe by design (probably will be added in future if any problems arise).

#### NetworkDataEntityQmlWrapper
Wrapper for NetworkDataEntity to use it in QML.

#### SimpleJsonAmqpClient
Base class to receive data from RabbitMQ APIs.

#### AmqpPublisher
Class for sending data to RabbitMQ.

#### AbstractRestServer
Base class for all HTTP-based web services. Any protected slots in derived class named in specific pattern will be considered as end points. Name should be `rest_<SCHEME>_<ENDPOINT>` where scheme is get/post/put/etc. and endpoint is anything which will be considered as endpoint. Underscores in endpoint name are treated as /, snake case is considered as delimitered with dash. E.g. `rest_get_System_RecentErrors` is treated as GET /system/recent-errors. Endpoint methods should accept next arguments:
 * `QTcpSocket *socket` - socket where method should write response after it is finished
 * `const QStringList &headers` - request headers
 * `const QStringList &methodVariableParts` - all url parts after ones contained in method name
 * `const QUrlQuery &query` - url query arguments
 * `const QByteArray &body` - request body

Contains two endpoints by itself:
 * GET /system/status returns combined info about service (can be extended by overriding `healthStatus()` method)
 * GET /system/recent-errors returns recent errors registered in in-memory error storage.

#### SmtpClient
Basic SMTP client, supports STARTTLS and SSL.

#### EmailNotificationHandler
Addition to ErrorNotifier from Core that allows to send errors via email.
