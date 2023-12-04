// ignore_for_file: avoid_print

import 'dart:convert';
import 'dart:math';
import 'package:mobx/mobx.dart';
import 'dart:io';
import 'dart:async';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import 'package:chalkdart/chalk.dart';
part 'mqtt_store.g.dart';

class MqttController = MqttControllerBase with _$MqttController;

abstract class MqttControllerBase with Store {
  @observable
  String macAdrr = '';

  @observable
  String user = '';

  @observable
  String password = '';

  @observable
  String authResult = '';

  MqttServerClient? _client;

  String _identifier = '';
  @observable
  String host = '';

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  @observable
  List<Map> eventsQueueJson = ObservableList<Map>();

  @observable
  List<Map> rawQueue = ObservableList<Map>();

  @observable
  bool isCheckedRememberMe = false;

  @action
  void toggleRemebermMe(bool value) {
    isCheckedRememberMe = value;
  }

  Future<void> initializeMQTTClient() async {
    Random random = Random();
    int randomNumber = random.nextInt(10000);
    _identifier = user + randomNumber.toString();

    _client = MqttServerClient(host, _identifier);
    _client!.setProtocolV31();
    _client!.port = 1883;
    _client!.keepAlivePeriod = 60;
    _client!.onDisconnected = onDisconnected;
    _client!.logging(on: false);
    _client!.onConnected = onConnected;
    _client!.onSubscribed = onSubscribed;
    _client!.autoReconnect = true;

    final connMess = MqttConnectMessage().authenticateAs(user, password).withClientIdentifier(_identifier).startClean().withWillQos(MqttQos.atLeastOnce);
    _client!.connectionMessage = connMess;

    print(chalk.keyword('orange')('CONNECTING...'));
  }

  Future<bool> connect() async {
    initializeMQTTClient();

    try {
      await _client!.connect();
    } on NoConnectionException {
      authResult = 'dados incorretos, tente novamente';
      return false;
    } on SocketException {
      authResult = 'dados incorretos, tente novamente';
      return false;
    }

    if (_client!.connectionStatus!.state == MqttConnectionState.connected) {
      authResult = '';
      return true;
    } else {
      authResult = 'dados incorretos, tente novamente';
      return false;
    }
  }

  void disconnect() {
    _client!.disconnect();

    print(chalk.keyword('orange')('DISCONNECTING...'));
  }

  void publish(String message) {
    final MqttClientPayloadBuilder builder = MqttClientPayloadBuilder();
    builder.addString(message);
    _client!.publishMessage("/", MqttQos.exactlyOnce, builder.payload!);
    print(chalk.keyword('orange')('PUB!'));
  }

  void onSubscribed(String topic) {
    print('onSubscribed::Subscription confirmed for topic $topic');
  }

  void onDisconnected() {
    print('onDisconnected::OnDisconnected _client callback - Client disconnection');
    if (_client!.connectionStatus!.disconnectionOrigin == MqttDisconnectionOrigin.solicited) {
      print('onDisconnected::OnDisconnected callback is solicited, this is correct');
    } else {
      print('onDisconnected::OnDisconnected callback is unsolicited or none, this is incorrect - exiting');
      print(_client!.connectionStatus!.disconnectionOrigin);
    }
  }

  void onConnected() {
    _client!.subscribe('events/$user', MqttQos.atLeastOnce);

    _client!.updates!.listen((List<MqttReceivedMessage<MqttMessage?>>? msg) {
      final data = msg![0].payload as MqttPublishMessage;
      final topic = msg[0].topic;
      final payload = MqttPublishPayload.bytesToStringAsString(data.payload.message);

      //var jsonObject = jsonDecode(payload);

      //addItemQueue(jsonObject["machine_id"], payload);

      print(chalk.black.onBrightWhite.bold('$topic' ' --> ' '$payload'));
    });
  }
}
