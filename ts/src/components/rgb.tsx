import * as React from 'react';

export interface SelectorProps {
  title: string;
  snitch?: (value: number) => void;
};

export interface SelectorState {
  value: number;
  error: string;
};

export interface ErrorProps {
  message: string;
};

class RGBSelector extends React.Component {
  props: SelectorProps;
  state: SelectorState;

  constructor(props: SelectorProps) {
    super(props);
    this.state = {
      value: 255,
      error: '',
    };
  }

  onChange = (event: any) => {
    let newValue = Number(event.target.value);
    if (isNaN(newValue) || newValue < 1 || newValue > 255) {
      this.setState({ error: 'RGB value must be a number between 1 and 255' });
    } else {
      this.setState({ error: ''});
      this.setState({ value: newValue });
      if (this.props.snitch) {
        this.props.snitch(newValue);
      }
    }
  }

  render() {
    const { title } = this.props;
    const { error } = this.state;
    return (
      <div>
        <div>{title}</div>
        <input
        type="text"
        value={this.state.value}
        placeholder="255"
        className="rgb-input"
        onChange={ (event) => this.onChange(event) }>
        </input>
        <RGBError message={error} />
      </div>
    );
  }
}

class RGBError extends React.Component {
  props: ErrorProps;

  render() {
    return (
      <div className="errorDiv">
      {this.props.message}
      </div>
    );
  }
}

export default RGBSelector;
