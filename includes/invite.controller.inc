<?php

/**
 * Validity constants.
 */
define('INVITE_VALID', 1);
define('INVITE_WITHDRAWN', 2);
define('INVITE_USED', 3);
define('INVITE_EXPIRED', 4);


class InviteController extends EntityAPIController {
  public function create(array $values = array()) {
    global $user;
    $values += array(
//      'title' => '',
//      'description' => '',
      'created' => REQUEST_TIME,
      'expiry' => REQUEST_TIME + variable_get('invite_default_expiry_time', 30) * 24 * 60 * 60,
      'uid' => $user->uid,
      'invitee' => 0,
      'reg_code' => invite_generate_code(),
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
  /**
   * The user id of the profile owner.
   *
   * @var integer
   */
  public $uid;

  /**
   * Returns the user owning this profile.
   */
  public function inviter() {
    return user_load($this->uid);
  }

  public function join_link() {
    return url('invite/accept/' . $this->reg_code, array('absolute' => TRUE));
  }

  /**
   * Sets a new user owning this profile.
   *
   * @param $account
   *   The user account object or the user account id (uid).
   */
  public function setUser($account) {
    $this->uid = is_object($account) ? $account->uid : $account;
  }

  public function __construct($values = array()) {
    if (isset($values['inviter'])) {
      $this->setUser($values['inviter']);
      unset($values['inviter']);
    }
    if (!isset($values['label']) && isset($values['type']) && $type = invite_get_types($values['type'])) {
      // Initialize the label with the type label, so newly created profiles
      // have that as interim label.
      $values['label'] = $type->label;
    }
    parent::__construct($values, 'invite');
  }

  public function status() {
    if (!$this || $this->canceled != 0) {
      return INVITE_WITHDRAWN;
    }
    elseif ($this->joined != 0) {
      return INVITE_USED;
    }
    elseif ($this->expiry < REQUEST_TIME) {
      return INVITE_EXPIRED;
    }
    else {
      return INVITE_VALID;
    }
  }

  protected function defaultLabel() {
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

  /**
   * Array with the argument keys.
   *
   * This is used to allow creation of entity metadata properties based
   * on the argument keys.
   *
   * @see InviteMetadataController::entityPropertyInfo()
   *
   * @var array
   */
  public $argument_keys = array();

  public $argument = array();

  public function __construct($values = array()) {
    parent::__construct($values, 'invite_type');
  }

  function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }
}

/**
 * Generates a unique tracking code.
 *
 * @return
 *   An 10-digit unique tracking code.
 */
function invite_generate_code() {
  do {
    $reg_code = user_password(10);
    $result = (bool) db_query_range('SELECT reg_code FROM {invite} WHERE reg_code = :regcode', 0, 1, array(':regcode' => $reg_code))->fetchField();
  } while ($result);

  return $reg_code;
}
