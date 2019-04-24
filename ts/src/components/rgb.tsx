import * as React from 'react';

export interface Props {
  value: 255,
}

class RGBSelector extends React.Component {
  constructor(props: Props) {
    super(props);
    this.state = {
      value: 255,
    };
  }

  onChange = (event: any) => {
    let newValue = Number(event.target.value);
    if (isNaN(newValue) || newValue < 1 || newValue > 255) {
      throw new Error('RGB value must be a number between 1 and 255');
    }

    this.setState({ value: newValue });
  };

  render() {
    return (
      <input type="text" minLength=1 maxLength=3 size=3 placeholder="255" className="rgb-input" onChange={ (event) => this.props.onChange(event) }></input>
    );
  };
}

export default RGBSelector;
