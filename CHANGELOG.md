ProofBase Changelog
===================

## Not Released
#### Features
 * UpdateManager class removed
 * Add email notifier for slow network requests
 
#### Bug Fixing
 * --

## 0.18.12.24
#### Features
 * Network: Better network check and errors about it (Add check for network and internet availability)
 * No logging through notifiers is done anymore if application instance doesn't exist (either not created or already dead)

#### Bug Fixing
 * --

## 0.18.10.4
#### Features
 * Bye-bye, TaskChain

#### Bug Fixing
 * --

## 0.18.9.23
#### Features
 * Network: AbstractAmqpReceiver::newQueueBindingExchangeName and AbstractAmqpReceiver::newQueueBindingRoutingKeys properties as common approach for binding new queues instead of exchangeName/routingKeys pairs in each client
 * Proof::tasks::unbounded::TaskChain is gone
 * ErrorMessagesRegistry is now part of ProofNetwork lib
 * It is now possible to create app-based api support derived from proofservicerestapi
 * ErrorMessagesRegistry is public now
 * markDirty available in protected section of ProofObject now

#### Bug Fixing
 * --

## 0.18.8.10
#### Features
 * Q_D_CONST/Q_Q_CONST macro for any usages of const d-ptr/q-ptr
 * Network: AbstractRestApi class finally removed
 * Network: Temporary ApiCall class removed
 * Network: Signals from API classes removed
 * Network: AbstractRestServer::healthStatus now returns Future
 * 25-30% less overhead on logging
 * Network: It is possible now to set amqp exchange type for auto creation

#### Bug Fixing
 * --

## 0.18.7.6
#### Features
 * Network: Added HttpDownloader
 * Network: SimpleJsonAmqpClient
 * Network: RestClient methods now returns `CancelableFuture<QNetworkReply *>`
 * Network: RestClient now is not required to be in the same thread as Api class
 * Network: RestClient now queues requests to same hosts if more than 6 are already running
 * Network: BaseRestApi class as next-gen network base class (replacement for AbstractRestApi)
 * Network: chainedApiCall backward compatibility support for BaseRestApi-based api classes
 * ProofObject::emitError() is virtual now
 * Almost all QList usages replaced with QVector
 * BaseRestApi can now be subclassed without Proof private headers
 * NetworkDataEntity can now be subclassed without Proof private headers

#### Bug Fixing
 * Network: X-Client-Name is now sent with UMS auth too

## 0.18.4.12
#### Features
 * Network: 202 is back to successful codes
 * TaskChain::wait() refined a bit to make wait less CPU-intensive
 * TaskChain::touchTask() refined a bit to avoid waiting

#### Bug Fixing
 * Better future management in taskchain

## 0.18.3.14
#### Features
 * Core: Proof::tasks::unbounded::TaskChain as copy of old TaskChain
 * Core: TaskChain is now a thin wrapper on top of tasks for backward compatibility and is deprecated
 * Network: ApiCall as wrapper around REST API which provides Future as a result
 * Qt 5.10 is now min version supported
 * Copies of errorOccurred signal in ProofObject children removed

#### Bug Fixing
 * --

## 0.18.1.24
#### Features
 * 173703: Make exchanges in AmqpPublisher autocreatable
 * Network: better clients deregistering in AbstractRestServer for more efficient memory usage during heavy load

#### Bug Fixing
 * crash fix for case when rest server tries to answer to already dead client
 * crash fix for deleting non self-destroyable task chains

## Changelog for releases older than 2018 can be found in main proof repository
