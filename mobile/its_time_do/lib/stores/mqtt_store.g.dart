// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'mqtt_store.dart';

// **************************************************************************
// StoreGenerator
// **************************************************************************

// ignore_for_file: non_constant_identifier_names, unnecessary_brace_in_string_interps, unnecessary_lambdas, prefer_expression_function_bodies, lines_longer_than_80_chars, avoid_as, avoid_annotating_with_dynamic, no_leading_underscores_for_local_identifiers

mixin _$MqttController on MqttControllerBase, Store {
  late final _$macAdrrAtom =
      Atom(name: 'MqttControllerBase.macAdrr', context: context);

  @override
  String get macAdrr {
    _$macAdrrAtom.reportRead();
    return super.macAdrr;
  }

  @override
  set macAdrr(String value) {
    _$macAdrrAtom.reportWrite(value, super.macAdrr, () {
      super.macAdrr = value;
    });
  }

  late final _$userAtom =
      Atom(name: 'MqttControllerBase.user', context: context);

  @override
  String get user {
    _$userAtom.reportRead();
    return super.user;
  }

  @override
  set user(String value) {
    _$userAtom.reportWrite(value, super.user, () {
      super.user = value;
    });
  }

  late final _$passwordAtom =
      Atom(name: 'MqttControllerBase.password', context: context);

  @override
  String get password {
    _$passwordAtom.reportRead();
    return super.password;
  }

  @override
  set password(String value) {
    _$passwordAtom.reportWrite(value, super.password, () {
      super.password = value;
    });
  }

  late final _$authResultAtom =
      Atom(name: 'MqttControllerBase.authResult', context: context);

  @override
  String get authResult {
    _$authResultAtom.reportRead();
    return super.authResult;
  }

  @override
  set authResult(String value) {
    _$authResultAtom.reportWrite(value, super.authResult, () {
      super.authResult = value;
    });
  }

  late final _$hostAtom =
      Atom(name: 'MqttControllerBase.host', context: context);

  @override
  String get host {
    _$hostAtom.reportRead();
    return super.host;
  }

  @override
  set host(String value) {
    _$hostAtom.reportWrite(value, super.host, () {
      super.host = value;
    });
  }

  late final _$eventsQueueJsonAtom =
      Atom(name: 'MqttControllerBase.eventsQueueJson', context: context);

  @override
  List<Map<dynamic, dynamic>> get eventsQueueJson {
    _$eventsQueueJsonAtom.reportRead();
    return super.eventsQueueJson;
  }

  @override
  set eventsQueueJson(List<Map<dynamic, dynamic>> value) {
    _$eventsQueueJsonAtom.reportWrite(value, super.eventsQueueJson, () {
      super.eventsQueueJson = value;
    });
  }

  late final _$rawQueueAtom =
      Atom(name: 'MqttControllerBase.rawQueue', context: context);

  @override
  List<Map<dynamic, dynamic>> get rawQueue {
    _$rawQueueAtom.reportRead();
    return super.rawQueue;
  }

  @override
  set rawQueue(List<Map<dynamic, dynamic>> value) {
    _$rawQueueAtom.reportWrite(value, super.rawQueue, () {
      super.rawQueue = value;
    });
  }

  late final _$isCheckedRememberMeAtom =
      Atom(name: 'MqttControllerBase.isCheckedRememberMe', context: context);

  @override
  bool get isCheckedRememberMe {
    _$isCheckedRememberMeAtom.reportRead();
    return super.isCheckedRememberMe;
  }

  @override
  set isCheckedRememberMe(bool value) {
    _$isCheckedRememberMeAtom.reportWrite(value, super.isCheckedRememberMe, () {
      super.isCheckedRememberMe = value;
    });
  }

  late final _$MqttControllerBaseActionController =
      ActionController(name: 'MqttControllerBase', context: context);

  @override
  void toggleRemebermMe(bool value) {
    final _$actionInfo = _$MqttControllerBaseActionController.startAction(
        name: 'MqttControllerBase.toggleRemebermMe');
    try {
      return super.toggleRemebermMe(value);
    } finally {
      _$MqttControllerBaseActionController.endAction(_$actionInfo);
    }
  }

  @override
  String toString() {
    return '''
macAdrr: ${macAdrr},
user: ${user},
password: ${password},
authResult: ${authResult},
host: ${host},
eventsQueueJson: ${eventsQueueJson},
rawQueue: ${rawQueue},
isCheckedRememberMe: ${isCheckedRememberMe}
    ''';
  }
}
