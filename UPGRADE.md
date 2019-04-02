Upgrades
========
Changes that one must make to applications based on Proof between versions.

## Not released
#### IT issues
 * --

#### API modifications/removals/deprecations
 * --

#### Config changes
 * --

#### Migrations
 * --

## 0.19.4.2
#### IT issues
 * --

#### API modifications/removals/deprecations
 * ProofObject::call is ProofObject::safeCall now and calls method only if it is not in same thread
 * ProofObject::call introduced that always calls the method
 * Proof::runApiCall and Proof::RestApiError removed

#### Config changes
 * --

#### Migrations
 * --

## 0.19.2.7
#### IT issues
 * --

#### API modifications/removals/deprecations
 * UpdateManager removed
 * RestApiConsumer removed

#### Config changes
 * `slow_network_notifier` section added to configure where emails about slow network requests should be sent
 * `app_id` moved from `error_notifier` to general section

#### Migrations
 * `app_id` moved from `error_notifier` to general section

## 0.18.10.4
#### IT issues
 * --

#### API modifications/removals/deprecations
 * TaskChain finally removed

#### Config changes
 * --

#### Migrations
 * --

## 0.18.9.23
#### IT issues
 * --

#### API modifications/removals/deprecations
 * Proof::tasks::unbounded::TaskChain is gone

#### Config changes
 * --

#### Migrations
 * --
