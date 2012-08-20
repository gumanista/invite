<?php

class InviteController extends EntityAPIController {
  public function create(array $values = array()) {
    global $user;
    $values += array(
      'title' => '',
      'description' => '',
      'created' => REQUEST_TIME,
      'expiry' => REQUEST_TIME + variable_get('invite_default_expiry_time', 30 * 24 * 60 * 60),
      'uid' => $user->uid,
      'invitee' => 0,
    );
    return parent::create($values);
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $wrapper = entity_metadata_wrapper('invite', $entity);
    $content['author'] = array('#markup' => t('Created by: !author', array('!author' => $wrapper->uid->name->value(array('sanitize' => TRUE)))));

    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }
}

class InviteTypeController extends EntityAPIControllerExportable {
  public function create(array $values = array()) {
    $values += array(
      'label' => '',
      'description' => '',
    );
    return parent::create($values);
  }

  /**
   * Save Invite Type.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    parent::save($entity, $transaction);
    // Rebuild menu registry. We do not call menu_rebuild directly, but set
    // variable that indicates rebuild in the end.
    // @see _http://drupal.org/node/1399618
    variable_set('menu_rebuild_needed', TRUE);
  }
}

/**
 * UI controller for Task Type.
 */
class InviteTypeUIController extends EntityDefaultUIController {
  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = 'Manage Invite types.';
    return $items;
  }
}

/**
 * Invite class.
 */
class Invite extends Entity {
  protected function defaultLabel() {
    dpm($this);
    $entity = entity_metadata_wrapper('invite', $this);
    return $entity->field_invitation_email_address->value();
  }
  protected function defaultUri() {
    return array('path' => 'invite/' . $this->identifier());
  }
}

/**
 * Invite Type class.
 */
class InviteType extends Entity {
  public $type;
  public $label;
  public $weight = 0;
  public function __construct($values = array()) {
    parent::__construct($values, 'invite_type');
  }

  function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }
}
